#pragma once
#include <cmath>
#include "vendor/SFML/Graphics.hpp"

#define PI 3.14159265358979f

namespace helpers
{
	inline float angleBetweenPoints(const sf::Vector2f& p1, const sf::Vector2f& p2) {
		return std::atan2(p2.y - p1.y, p2.x - p1.x) * 180.0f / PI;
	}

	inline float lengthBetweenPoints(const sf::Vector2f& p1, const sf::Vector2f& p2) {
		return std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
	}

    inline void invertColors(sf::Image& image) {
        for (unsigned y = 0; y < image.getSize().y; ++y) {
            for (unsigned x = 0; x < image.getSize().x; ++x) {
                sf::Color color = image.getPixel(x, y);
                color.r = 255 - color.r;
                color.g = 255 - color.g;
                color.b = 255 - color.b;
                image.setPixel(x, y, color);
            }
        }
    }

    inline void InvertTexture(sf::Texture& texture)
    {
        auto image = texture.copyToImage();
        invertColors(image);
        texture.update(image);
    }

    std::vector<std::string>    split_string    (const std::string& input, const std::string& delimiters, uint16_t expected_vec_size = 16);
    std::string                 strip_str(const std::string& str);


}
