/**
 * @file imu9.h
 *
 * IMU9 (9 axes) Events format definition and handling functions.
 * This contains data coming from the Inertial Measurement Unit
 * chip, with the 3-axes accelerometer and 3-axes gyroscope.
 * Temperature is also included.
 * Further, 3-axes from the magnetometer are included, which
 * can be used to get a compass-like heading.
 */

#ifndef LIBCAER_EVENTS_IMU9_H_
#define LIBCAER_EVENTS_IMU9_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

/**
 * IMU 9-axes event data structure definition.
 * This contains accelerometer and gyroscope headings, plus
 * temperature, and magnetometer readings.
 * Floats are in IEEE 754-2008 binary32 format.
 * Signed integers are used for fields that are to be interpreted
 * directly, for compatibility with languages that do not have
 * unsigned integer types, such as Java.
 */
struct caer_imu9_event {
	// Event information. First because of valid mark.
	uint32_t info;
	// Event timestamp.
	int32_t timestamp;
	// Acceleration in the X axis, measured in g (9.81m/s²).
	float accel_x;
	// Acceleration in the Y axis, measured in g (9.81m/s²).
	float accel_y;
	// Acceleration in the Z axis, measured in g (9.81m/s²).
	float accel_z;
	// Rotation in the X axis, measured in °/s.
	float gyro_x;
	// Rotation in the Y axis, measured in °/s.
	float gyro_y;
	// Rotation in the Z axis, measured in °/s.
	float gyro_z;
	// Temperature, measured in °C.
	float temp;
	// Magnetometer X axis, measured in µT (magnetic flux density).
	float comp_x;
	// Magnetometer Y axis, measured in µT (magnetic flux density).
	float comp_y;
	// Magnetometer Z axis, measured in µT (magnetic flux density).
	float comp_z;
}__attribute__((__packed__));

/**
 * Type for pointer to IMU 9-axes event data structure.
 */
typedef struct caer_imu9_event *caerIMU9Event;

/**
 * IMU 9-axes event packet data structure definition.
 * EventPackets are always made up of the common packet header,
 * followed by 'eventCapacity' events. Everything has to
 * be in one contiguous memory block.
 */
struct caer_imu9_event_packet {
	// The common event packet header.
	struct caer_event_packet_header packetHeader;
	// The events array.
	struct caer_imu9_event events[];
}__attribute__((__packed__));

/**
 * Type for pointer to IMU 9-axes event packet data structure.
 */
typedef struct caer_imu9_event_packet *caerIMU9EventPacket;

caerIMU9EventPacket caerIMU9EventPacketAllocate(int32_t eventCapacity, int16_t eventSource, int32_t tsOverflow);

static inline caerIMU9Event caerIMU9EventPacketGetEvent(caerIMU9EventPacket packet, int32_t n) {
	// Check that we're not out of bounds.
	if (n < 0 || n >= caerEventPacketHeaderGetEventCapacity(&packet->packetHeader)) {
#if !defined(LIBCAER_LOG_NONE)
		caerLog(CAER_LOG_CRITICAL, "IMU9 Event",
			"Called caerIMU9EventPacketGetEvent() with invalid event offset %" PRIi32 ", while maximum allowed value is %" PRIi32 ".",
			n, caerEventPacketHeaderGetEventCapacity(&packet->packetHeader));
#endif
		return (NULL);
	}

	// Return a pointer to the specified event.
	return (packet->events + n);
}

static inline int32_t caerIMU9EventGetTimestamp(caerIMU9Event event) {
	return (le32toh(event->timestamp));
}

static inline int64_t caerIMU9EventGetTimestamp64(caerIMU9Event event, caerIMU9EventPacket packet) {
	return (I64T(
		(U64T(caerEventPacketHeaderGetEventTSOverflow(&packet->packetHeader)) << TS_OVERFLOW_SHIFT) | U64T(caerIMU9EventGetTimestamp(event))));
}

// Limit Timestamp to 31 bits for compatibility with languages that have no unsigned integer (Java).
static inline void caerIMU9EventSetTimestamp(caerIMU9Event event, int32_t timestamp) {
	if (timestamp < 0) {
		// Negative means using the 31st bit!
#if !defined(LIBCAER_LOG_NONE)
		caerLog(CAER_LOG_CRITICAL, "IMU9 Event", "Called caerIMU9EventSetTimestamp() with negative value!");
#endif
		return;
	}

	event->timestamp = htole32(timestamp);
}

static inline bool caerIMU9EventIsValid(caerIMU9Event event) {
	return ((le32toh(event->info) >> VALID_MARK_SHIFT) & VALID_MARK_MASK);
}

static inline void caerIMU9EventValidate(caerIMU9Event event, caerIMU9EventPacket packet) {
	if (!caerIMU9EventIsValid(event)) {
		event->info |= htole32(U32T(1) << VALID_MARK_SHIFT);

		// Also increase number of events and valid events.
		// Only call this on (still) invalid events!
		caerEventPacketHeaderSetEventNumber(&packet->packetHeader,
			caerEventPacketHeaderGetEventNumber(&packet->packetHeader) + 1);
		caerEventPacketHeaderSetEventValid(&packet->packetHeader,
			caerEventPacketHeaderGetEventValid(&packet->packetHeader) + 1);
	}
	else {
#if !defined(LIBCAER_LOG_NONE)
		caerLog(CAER_LOG_CRITICAL, "IMU9 Event", "Called caerIMU9EventValidate() on already valid event.");
#endif
	}
}

static inline void caerIMU9EventInvalidate(caerIMU9Event event, caerIMU9EventPacket packet) {
	if (caerIMU9EventIsValid(event)) {
		event->info &= htole32(~(U32T(1) << VALID_MARK_SHIFT));

		// Also decrease number of valid events. Number of total events doesn't change.
		// Only call this on valid events!
		caerEventPacketHeaderSetEventValid(&packet->packetHeader,
			caerEventPacketHeaderGetEventValid(&packet->packetHeader) - 1);
	}
	else {
#if !defined(LIBCAER_LOG_NONE)
		caerLog(CAER_LOG_CRITICAL, "IMU9 Event", "Called caerIMU9EventInvalidate() on already invalid event.");
#endif
	}
}

static inline float caerIMU9EventGetAccelX(caerIMU9Event event) {
	return (le32toh(event->accel_x));
}

static inline void caerIMU9EventSetAccelX(caerIMU9Event event, float accelX) {
	event->accel_x = htole32(accelX);
}

static inline float caerIMU9EventGetAccelY(caerIMU9Event event) {
	return (le32toh(event->accel_y));
}

static inline void caerIMU9EventSetAccelY(caerIMU9Event event, float accelY) {
	event->accel_y = htole32(accelY);
}

static inline float caerIMU9EventGetAccelZ(caerIMU9Event event) {
	return (le32toh(event->accel_z));
}

static inline void caerIMU9EventSetAccelZ(caerIMU9Event event, float accelZ) {
	event->accel_z = htole32(accelZ);
}

static inline float caerIMU9EventGetGyroX(caerIMU9Event event) {
	return (le32toh(event->gyro_x));
}

static inline void caerIMU9EventSetGyroX(caerIMU9Event event, float gyroX) {
	event->gyro_x = htole32(gyroX);
}

static inline float caerIMU9EventGetGyroY(caerIMU9Event event) {
	return (le32toh(event->gyro_y));
}

static inline void caerIMU9EventSetGyroY(caerIMU9Event event, float gyroY) {
	event->gyro_y = htole32(gyroY);
}

static inline float caerIMU9EventGetGyroZ(caerIMU9Event event) {
	return (le32toh(event->gyro_z));
}

static inline void caerIMU9EventSetGyroZ(caerIMU9Event event, float gyroZ) {
	event->gyro_z = htole32(gyroZ);
}

static inline float caerIMU9EventGetCompX(caerIMU9Event event) {
	return (le32toh(event->comp_x));
}

static inline void caerIMU9EventSetCompX(caerIMU9Event event, float compX) {
	event->comp_x = htole32(compX);
}

static inline float caerIMU9EventGetCompY(caerIMU9Event event) {
	return (le32toh(event->comp_y));
}

static inline void caerIMU9EventSetCompY(caerIMU9Event event, float compY) {
	event->comp_y = htole32(compY);
}

static inline float caerIMU9EventGetCompZ(caerIMU9Event event) {
	return (le32toh(event->comp_z));
}

static inline void caerIMU9EventSetCompZ(caerIMU9Event event, float compZ) {
	event->comp_z = htole32(compZ);
}

static inline float caerIMU9EventGetTemp(caerIMU9Event event) {
	return (le32toh(event->temp));
}

static inline void caerIMU9EventSetTemp(caerIMU9Event event, float temp) {
	event->temp = htole32(temp);
}

#ifdef __cplusplus
}
#endif

#endif /* LIBCAER_EVENTS_IMU9_H_ */
