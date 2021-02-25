#pragma once

#include <DirectXMath.h>
#include <memory>
#include <map>

namespace Cyrex {
    class Texture;
    struct alignas(16) MaterialProperties {
        MaterialProperties(
            const DirectX::XMFLOAT4 diffuse     = { 1.0f, 1.0f, 1.0f, 1.0f },
            const DirectX::XMFLOAT4 specular    = { 1.0f, 1.0f, 1.0f, 1.0f },
            const float specularPower           = 128.0f,
            const DirectX::XMFLOAT4 ambient     = { 0.0f, 0.0f, 0.0f, 1.0f },
            const DirectX::XMFLOAT4 emissive    = { 0.0f, 0.0f, 0.0f, 1.0f },
            const DirectX::XMFLOAT4 reflectance = { 0.0f, 0.0f, 0.0f, 0.0f },
            const float opacity                 = 1.0f,
            const float indexOfRefraction       = 0.0f,
            const float bumpIntensity           = 1.0f,
            const float alphaThreshHold         = 0.1f
        )
            :
            Diffuse(diffuse),
            Specular(specular),
            Emissive(emissive),
            Ambient(ambient),
            Reflectance(reflectance),
            Opacity(opacity),
            SpecularPower(specularPower),
            IndexOfRefraction(indexOfRefraction),
            BumpIntensity(bumpIntensity),
            HasAmbientTexture(false),
            HasEmissiveTexture(false),
            HasDiffuseTexture(false),
            HasSpecularTexture(false),
            HasSpecularPowerTexture(false),
            HasNormalTexture(false),
            HasBumpTexture(false),
            HasOpacityTexture(false)
        {}

        DirectX::XMFLOAT4 Diffuse;
        DirectX::XMFLOAT4 Specular;
        DirectX::XMFLOAT4 Emissive;
        DirectX::XMFLOAT4 Ambient;
        DirectX::XMFLOAT4 Reflectance;

        float Opacity;
        float SpecularPower;
        float IndexOfRefraction;
        float BumpIntensity;


        uint32_t HasAmbientTexture;
        uint32_t HasEmissiveTexture;
        uint32_t HasDiffuseTexture;
        uint32_t HasSpecularTexture;

        uint32_t HasSpecularPowerTexture;
        uint32_t HasNormalTexture;
        uint32_t HasBumpTexture;
        uint32_t HasOpacityTexture;
    };

    class Material {
    public:
        enum TextureType {
            Ambient,
            Emissive,
            Diffuse,
            Specular,
            SpecularPower,
            Normal,
            Bump,
            Opacity,
            NumTypes
        };

        Material(const MaterialProperties& materialProperties = MaterialProperties());
        Material(const Material& rhs);
        ~Material() = default;

        const DirectX::XMFLOAT4& GetAmbientColor() const noexcept;
        void SetAmbientColor(const DirectX::XMFLOAT4& ambient) noexcept;

        const DirectX::XMFLOAT4& GetDiffuseColor() const noexcept;
        void SetDiffuseColor(const DirectX::XMFLOAT4& diffuse) noexcept;

        const DirectX::XMFLOAT4& GetSpecularColor() const noexcept;
        void SetSpecularColor(const DirectX::XMFLOAT4& specular) noexcept;

        const DirectX::XMFLOAT4& GetEmissiveColor() const noexcept;
        void SetEmissiveColor(const DirectX::XMFLOAT4& emissive) noexcept;

        float GetSpecularPower() const noexcept;
        void SetSpecularPower(float specularPower) noexcept;

        const DirectX::XMFLOAT4& GetReflectance() const noexcept;
        void SetReflectance(const DirectX::XMFLOAT4& reflectance) noexcept;

        const float GetOpacity() const noexcept;
        void SetOpacity(float opacity) noexcept;

        float GetIndexOfRefraction() const noexcept;
        void SetIndexOfRefraction(float indexOfRefraction) noexcept;

        float GetBumbIntensity() const;
        void SetBumpIntensity(float bumbIntensity);


        std::shared_ptr<Texture> GetTexture(TextureType ID) const noexcept;
        void SetTexture(TextureType type, std::shared_ptr<Texture> texture) noexcept;

        bool IsTransparent() const noexcept;

        const MaterialProperties GetMaterialProperties() const noexcept;
        void SetMaterialProperties(const MaterialProperties& materialProperties) noexcept;

        // Define some materials.
        static const MaterialProperties Zero;
        static const MaterialProperties Red;
        static const MaterialProperties Green;
        static const MaterialProperties Blue;
        static const MaterialProperties Cyan;
        static const MaterialProperties Magenta;
        static const MaterialProperties Yellow;
        static const MaterialProperties White;
        static const MaterialProperties WhiteDiffuse;
        static const MaterialProperties Black;
        static const MaterialProperties Emerald;
        static const MaterialProperties Jade;
        static const MaterialProperties Obsidian;
        static const MaterialProperties Pearl;
        static const MaterialProperties Ruby;
        static const MaterialProperties Turquoise;
        static const MaterialProperties Brass;
        static const MaterialProperties Bronze;
        static const MaterialProperties Chrome;
        static const MaterialProperties Copper;
        static const MaterialProperties Gold;
        static const MaterialProperties Silver;
        static const MaterialProperties BlackPlastic;
        static const MaterialProperties CyanPlastic;
        static const MaterialProperties GreenPlastic;
        static const MaterialProperties RedPlastic;
        static const MaterialProperties WhitePlastic;
        static const MaterialProperties YellowPlastic;
        static const MaterialProperties BlackRubber;
        static const MaterialProperties CyanRubber;
        static const MaterialProperties GreenRubber;
        static const MaterialProperties RedRubber;
        static const MaterialProperties WhiteRubber;
        static const MaterialProperties YellowRubber;
    private:
        using TextureMap = std::map<TextureType, std::shared_ptr<Texture>>;

        std::unique_ptr<MaterialProperties> m_materialProperties;
        TextureMap m_textures;
    };
}
