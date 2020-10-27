#include "../src/hdcamera.h"
#include "../src/config.h"
#include <stdio.h>
#include <catch2/catch.hpp>


TEST_CASE("example", "[example]"){
	int status = SUCCESS;
	printf("IN TEST DSFKALJFDSAKLF;DJSAKL;FDSAL;\n");
	HDCamera camera = HDCamera(&status, DEFAULT_ETHERNET_PACKET_SIZE);
	REQUIRE( 3 == 3);
}
