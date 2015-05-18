// Copyright 2014 http://switchdevice.com
// This example code is in the public domain.

#include "gtest/gtest.h"
#include "arduino-mock/Arduino.h"
#include "src/gbprinter.h"

using ::testing::Return;

TEST(cbuffer, initialization) {
	EXPECT_EQ(0u, CBUFFER.start);
	EXPECT_EQ(0u, CBUFFER.end);
}

TEST(cbuffer, cbread) {
	CBUFFER.buffer[0] = 0x01;
	CBUFFER.buffer[1] = 0x02;
	// CBRead reads properly from buffer and advances start pointer
	EXPECT_EQ(0x01, CBRead(&CBUFFER));
	EXPECT_EQ(0x02, CBRead(&CBUFFER));
	// CBRead loops over at the end of the buffer
	CBUFFER.start = BUFFER_SIZE - 1;
	CBRead(&CBUFFER);
	EXPECT_EQ(0u, CBUFFER.start);
}

TEST(cbuffer, cbwrite) {
	// Check that write actually does it and advances buffer accordingly
	CBWrite(&CBUFFER, 0x02);
	CBWrite(&CBUFFER, 0x03);
	EXPECT_EQ(0x02, CBUFFER.buffer[0]);
	EXPECT_EQ(0x03, CBUFFER.buffer[1]);
	// Check that end loops over at the end of the bugger
	CBUFFER.end = BUFFER_SIZE -1;
	CBWrite(&CBUFFER, 0x03);
	CBWrite(&CBUFFER, 0x03);
	EXPECT_EQ(0x03, CBUFFER.buffer[BUFFER_SIZE - 1]);
	EXPECT_EQ(0x03, CBUFFER.buffer[0]);
}
