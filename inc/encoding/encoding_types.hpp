#pragma once

#include <string_view>
#include <utility>

enum EncoderType { 
	NO_ENCODING, 
	MORTON_ENCODER_3D, 
	MORTON_ENCODER_2D_X, 
	MORTON_ENCODER_2D_Y, 
	MORTON_ENCODER_2D_Z, 
	HILBERT_ENCODER_3D, 
	HILBERT_ENCODER_2D_X, 
	HILBERT_ENCODER_2D_Y,
	HILBERT_ENCODER_2D_Z 
};

constexpr std::pair<EncoderType, std::string_view> encoderTypeMap[] = {
    { EncoderType::NO_ENCODING,       		"none" },
    { EncoderType::MORTON_ENCODER_3D,   	"mort" },
    { EncoderType::MORTON_ENCODER_2D_X,   	"mort_2d_x" },
    { EncoderType::MORTON_ENCODER_2D_Y,   	"mort_2d_y" },
    { EncoderType::MORTON_ENCODER_2D_Z,   	"mort_2d_z" },
    { EncoderType::HILBERT_ENCODER_3D,  	"hilb" },
    { EncoderType::HILBERT_ENCODER_2D_X,   	"hilb_2d_x" },
    { EncoderType::HILBERT_ENCODER_2D_Y,   	"hilb_2d_y" },
    { EncoderType::HILBERT_ENCODER_2D_Z,   	"hilb_2d_z" },
};

constexpr std::string_view encoderTypeToString(EncoderType enc) {
    for (const auto& [key, val] : encoderTypeMap) {
        if (key == enc) return val;
    }
    return "Unknown";
}