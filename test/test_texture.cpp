#include <iostream>
#include <vector>
#include <fstream>
#include "../PerlinNoise.cpp"

// PerlinNoise 类的定义和实现应放在这里

const int noiseWidth = 1024; // 纹理宽度
const int noiseHeight = 1024; // 纹理高度

std::vector<float> noiseData(noiseWidth * noiseHeight); // 用于存储噪声数据

// 生成Perlin噪声的函数
void generatePerlinNoise() {
    PerlinNoise perlin(3); // 使用你的 PerlinNoise 类实例化

    for (int y = 0; y < noiseHeight; ++y) {
        for (int x = 0; x < noiseWidth; ++x) {
            double nx = static_cast<double>(x) / noiseWidth;
            double ny = static_cast<double>(y) / noiseHeight;
            double noiseValue = perlin.generateNoise(nx * 5.0, ny * 5.0, 0.5, 2.0, 0.6, 3, 0.5, 0.6);
            if (noiseValue < 0.00 && noiseValue > -0.001)
                noiseData[y * noiseWidth + x] = 1.0;
            else
                noiseData[y * noiseWidth + x] = 0.0f;
        }
    }
}

// 创建沙地纹理并叠加Perlin噪声
std::vector<unsigned char> createSandTextureWithNoise() {
    const int width = noiseWidth;
    const int height = noiseHeight;
    std::vector<unsigned char> textureData(width * height * 3);

    // 沙地纯色RGB值
    const unsigned char sandR = 240;
    const unsigned char sandG = 215;
    const unsigned char sandB = 148;

    // 生成沙地纹理并叠加Perlin噪声
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float noiseValue = noiseData[y * width + x];
            int index = (y * width + x) * 3;
            textureData[index] = static_cast<unsigned char>(sandR * (1.0f - noiseValue));
            textureData[index + 1] = static_cast<unsigned char>(sandG * (1.0f - noiseValue));
            textureData[index + 2] = static_cast<unsigned char>(sandB * (1.0f - noiseValue));
        }
    }

    return textureData;
}

// 保存PNM文件
void savePNM(const std::vector<unsigned char>& textureData, const std::string& filename, int width, int height) {
    std::ofstream file(filename, std::ios::binary);

    file << "P6\n" << width << " " << height << "\n255\n";
    file.write(reinterpret_cast<const char*>(textureData.data()), textureData.size());

    file.close();
}

int main() {
    // 生成Perlin噪声
    generatePerlinNoise();

    // 创建沙地纹理并叠加Perlin噪声
    std::vector<unsigned char> sandTexture = createSandTextureWithNoise();

    // 保存为PNM文件
    savePNM(sandTexture, "sand_texture_with_noise.pnm", noiseWidth, noiseHeight);

    std::cout << "Generated sand texture with noise and saved to sand_texture_with_noise.pnm" << std::endl;

    return 0;
}

