#include "../src/hdcamera.h"
#include "../src/config.h"
#include <stdio.h>
#include <arv.h>
#include <catch2/catch.hpp>


TEST_CASE("test camera", "[test]") {
	char serial[] = "1345678";
	ArvCamera *arvCamera = ARV_CAMERA(arv_fake_camera_new(serial));

	ArvStream *stream = arv_camera_create_stream (arvCamera, NULL, NULL, NULL);
	if (!ARV_IS_STREAM (stream)) {
		printf ("Stream thread was unintialized (check if the device is not already used, or if stream was configured)\n");
	}
}
