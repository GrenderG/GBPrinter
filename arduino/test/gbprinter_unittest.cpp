// Copyright 2014 http://switchdevice.com
// This example code is in the public domain.

#include "gtest/gtest.h"
#include "arduino-mock/Arduino.h"
#include "src/gbprinter.h"

using ::testing::Return;

TEST(cbuffer, initialization) {
	CBuffer cb = { .start = 0, .end= 0 };	
	EXPECT_EQ(0u, cb.start);
	EXPECT_EQ(0u, cb.end);
}

TEST(cbuffer, cbread) {
	CBuffer cb = { .start = 0, .end= 0 };
	cb.buffer[0] = 0x01;
	cb.buffer[1] = 0x02;
	// CBRead reads properly from buffer and advances start pointer
	EXPECT_EQ(0x01, CBRead(&cb));
	EXPECT_EQ(0x02, CBRead(&cb));
	// CBRead loops over at the end of the buffer
	cb.start = BUFFER_SIZE - 1;
	CBRead(&cb);
	EXPECT_EQ(0u, cb.start);
}

TEST(cbuffer, cbwrite) {
	CBuffer cb = { .start = 0, .end= 0 };
	CBWrite(&cb, 0x02);
	CBWrite(&cb, 0x03);
	EXPECT_EQ(0x02, cb.buffer[0]);
	EXPECT_EQ(0x03, cb.buffer[1]);
	cb.end = BUFFER_SIZE -1;
	CBWrite(&cb, 0x03);
	CBWrite(&cb, 0x03);
	EXPECT_EQ(0x03, cb.buffer[BUFFER_SIZE - 1]);
	EXPECT_EQ(0x03, cb.buffer[0]);
}
