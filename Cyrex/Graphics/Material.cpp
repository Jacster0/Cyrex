#include "Material.h"

static Cyrex::MaterialProperties* NewMaterialProperties(const Cyrex::MaterialProperties& props) {
    Cyrex::MaterialProperties* materialProperties = 
        static_cast<Cyrex::MaterialProperties*>(_aligned_malloc(sizeof(Cyrex::MaterialProperties), 16));

    return materialProperties;
}

static void DeleteMaterialProperties(Cyrex::MaterialProperties* materialProperties) {
    _aligned_free(materialProperties);
}

Cyrex::Material::Material(const MaterialProperties& materialProperties)
    :
    m_materialProperties(NewMaterialProperties(materialProperties), &DeleteMaterialProperties)
{
}

Cyrex::Material::Material(const Material& rhs)
    :
    m_materialProperties(NewMaterialProperties(*rhs.m_materialProperties), &DeleteMaterialProperties),
    m_textures(rhs.m_textures)
{
}

const DirectX::XMFLOAT4& Cyrex::Material::GetAmbientColor() const noexcept {
    return m_materialProperties->Ambient;
}

void Cyrex::Material::SetAmbientColor(const DirectX::XMFLOAT4& ambient) noexcept {
    m_materialProperties->Ambient = ambient;
}

const DirectX::XMFLOAT4& Cyrex::Material::GetDiffuseColor() const noexcept {
    return m_materialProperties->Diffuse;
}

void Cyrex::Material::SetDiffuseColor(const DirectX::XMFLOAT4& diffuse) noexcept {
    m_materialProperties->Diffuse = diffuse;
}

const DirectX::XMFLOAT4& Cyrex::Material::GetSpecularColor() const noexcept {
    return m_materialProperties->Specular;
}

void Cyrex::Material::SetSpecularColor(const DirectX::XMFLOAT4& specular) noexcept {
    m_materialProperties->Specular = specular;
}

const DirectX::XMFLOAT4& Cyrex::Material::GetEmissiveColor() const noexcept {
    return m_materialProperties->Emissive;
}

void Cyrex::Material::SetEmissiveColor(const DirectX::XMFLOAT4& emissive) noexcept {
    m_materialProperties->Emissive = emissive;
}

float Cyrex::Material::GetSpecularPower() const noexcept {
    return m_materialProperties->SpecularPower;
}

void Cyrex::Material::SetSpecularPower(float specularPower) noexcept {
    m_materialProperties->SpecularPower = specularPower;
}

const DirectX::XMFLOAT4& Cyrex::Material::GetReflectance() const noexcept {
    return m_materialProperties->Reflectance;
}

void Cyrex::Material::SetReflectance(const DirectX::XMFLOAT4& reflectance) noexcept {
    m_materialProperties->Reflectance = reflectance;
}

const float Cyrex::Material::GetOpacity() const noexcept {
    return m_materialProperties->Opacity;
}

void Cyrex::Material::SetOpacity(float opacity) noexcept {
    m_materialProperties->Opacity = opacity;
}

float Cyrex::Material::GetIndexOfRefraction() const noexcept {
    return m_materialProperties->IndexOfRefraction;
}

void Cyrex::Material::SetIndexOFRefraction(float indexOfRefraction) noexcept {
    m_materialProperties->IndexOfRefraction = indexOfRefraction;
}

float Cyrex::Material::GetBumbIntensity() const {
    return m_materialProperties->BumpIntensity;
}

void Cyrex::Material::SetBumpIntenisty(float bumbIntensity) {
    m_materialProperties->BumpIntensity = bumbIntensity;
}

std::shared_ptr<Cyrex::Texture> Cyrex::Material::GetTexture(TextureType ID) const noexcept {
    TextureMap::const_iterator iter = m_textures.find(ID);

    if (iter != m_textures.end()) {
        return iter->second;
    }
    return nullptr;
}

void Cyrex::Material::SetTexture(TextureType type, std::shared_ptr<Texture> texture) noexcept {
    m_textures[type] = texture;

    switch (type)
    {
    case Cyrex::Material::TextureType::Ambient:
        m_materialProperties->HasAmbientTexture = (texture != nullptr);
        break;
    case Cyrex::Material::TextureType::Emissive:
        m_materialProperties->HasEmissiveTexture = (texture != nullptr);
        break;
    case Cyrex::Material::TextureType::Diffuse:
        m_materialProperties->HasDiffuseTexture = (texture != nullptr);
        break;
    case Cyrex::Material::TextureType::Specular:
        m_materialProperties->HasSpecularTexture = (texture != nullptr);
        break;
    case Cyrex::Material::TextureType::SpecularPower:
        m_materialProperties->HasSpecularPowerTexture = (texture != nullptr);
        break;
    case Cyrex::Material::TextureType::Normal:
        m_materialProperties->HasNormalTexture = (texture != nullptr);
        break;
    case Cyrex::Material::TextureType::Bump:
        m_materialProperties->HasBumpTexture = (texture != nullptr);
        break;
    case Cyrex::Material::TextureType::Opacity:
        m_materialProperties->HasOpacityTexture = (texture != nullptr);
        break;
    case Cyrex::Material::TextureType::NumTypes:
        break;
    }
}

bool Cyrex::Material::IsTransaprent() const noexcept {
    return (m_materialProperties->Opacity < 1.0f || m_materialProperties->HasOpacityTexture);
}

const Cyrex::MaterialProperties Cyrex::Material::GetMaterialProperties() const noexcept {
    return *m_materialProperties;
}

void Cyrex::Material::SetMaterialProperties(const MaterialProperties& materialProperties) noexcept {
    *m_materialProperties = materialProperties;
}

const Cyrex::MaterialProperties Cyrex::Material::Zero = {
    { 0.0f, 0.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 0.0f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::Red = {
    { 1.0f, 0.0f, 0.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f },
    128.0f,
    { 0.1f, 0.0f, 0.0f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::Green = {
    { 0.0f, 1.0f, 0.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f },
    128.0f,
    { 0.0f, 0.1f, 0.0f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::Blue = {
    { 0.0f, 0.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f },
    128.0f,
    { 0.0f, 0.0f, 0.1f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::Cyan = {
    { 0.0f, 1.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f },
    128.0f,
    { 0.0f, 0.1f, 0.1f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::Magenta = {
    { 1.0f, 0.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f },
    128.0f,
    { 0.1f, 0.0f, 0.1f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::Yellow = {
    { 0.0f, 1.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f },
    128.0f,
    { 0.0f, 0.1f, 0.1f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::White = {
    { 1.0f, 1.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f },
    128.0f,
    { 0.1f, 0.1f, 0.1f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::WhiteDiffuse = {
    { 1.0f, 1.0f, 1.0f, 1.0f },
    { 0.0f, 0.0f, 0.0f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::Black = {
    { 0.0f, 0.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 0.0f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::Emerald = {
    { 0.07568f, 0.61424f,  0.07568f, 1.0f },
    { 0.633f,   0.727811f, 0.633f,   1.0f },
    76.8f,
    { 0.0215f, 0.1745f, 0.0215f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::Jade = {
    { 0.54f,     0.89f,     0.63f,     1.0f },
    { 0.316228f, 0.316228f, 0.316228f, 1.0f },
    12.8f,
    { 0.135f, 0.2225f, 0.1575f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::Obsidian = {
    { 0.18275f,  0.17f,     0.22525f,  1.0f },
    { 0.332741f, 0.328634f, 0.346435f, 1.0f },
    38.4f,
    { 0.05375f, 0.05f, 0.06625f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::Pearl = {
    { 1.0f,      0.829f,    0.829f,    1.0f },
    { 0.296648f, 0.296648f, 0.296648f, 1.0f },
    11.264f,
    { 0.25f, 0.20725f, 0.20725f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::Ruby = {
    { 0.61424f,  0.04136f,  0.04136f,  1.0f },
    { 0.727811f, 0.626959f, 0.626959f, 1.0f },
    76.8f,
    { 0.1745f, 0.01175f, 0.01175f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::Turquoise = {
    { 0.396f,    0.74151f, 0.69102f,  1.0f },
    { 0.297254f, 0.30829f, 0.306678f, 1.0f },
    12.8f,
    { 0.1f, 0.18725f, 0.1745f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::Brass = {
    { 0.780392f, 0.568627f, 0.113725f, 1.0f },
    { 0.992157f, 0.941176f, 0.807843f, 1.0f },
    27.9f,
    { 0.329412f, 0.223529f, 0.027451f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::Bronze = {
    { 0.714f,    0.4284f,   0.18144f,  1.0f },
    { 0.393548f, 0.271906f, 0.166721f, 1.0f },
    25.6f,
    { 0.2125f, 0.1275f, 0.054f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::Chrome = {
    { 0.4f,      0.4f,      0.4f,      1.0f },
    { 0.774597f, 0.774597f, 0.774597f, 1.0f },
    76.8f,
    { 0.25f, 0.25f, 0.25f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::Copper = {
    { 0.7038f,   0.27048f,  0.0828f,   1.0f },
    { 0.256777f, 0.137622f, 0.086014f, 1.0f },
    12.8f,
    { 0.19125f, 0.0735f, 0.0225f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::Gold = {
    { 0.75164f,  0.60648f,  0.22648f,  1.0f },
    { 0.628281f, 0.555802f, 0.366065f, 1.0f },
    51.2f,
    { 0.24725f, 0.1995f, 0.0745f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::Silver = {
    { 0.50754f,  0.50754f,  0.50754f,  1.0f },
    { 0.508273f, 0.508273f, 0.508273f, 1.0f },
    51.2f,
    { 0.19225f, 0.19225f, 0.19225f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::BlackPlastic = {
    { 0.01f, 0.01f, 0.01f, 1.0f },
    { 0.5f,  0.5f,  0.5f,  1.0f },
    32.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::CyanPlastic = {
    { 0.0f,        0.50980392f, 0.50980392f, 1.0f },
    { 0.50196078f, 0.50196078f, 0.50196078f, 1.0f },
    32.0f,
    { 0.0f, 0.1f, 0.06f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::GreenPlastic = {
    { 0.1f,  0.35f, 0.1f,  1.0f },
    { 0.45f, 0.55f, 0.45f, 1.0f },
    32.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::RedPlastic = {
    { 0.5f, 0.0f, 0.0f, 1.0f },
    { 0.7f, 0.6f, 0.6f, 1.0f },
    32.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::WhitePlastic = {
    { 0.55f, 0.55f, 0.55f, 1.0f },
    { 0.7f,  0.7f,  0.7f,  1.0f },
    32.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::YellowPlastic = {
    { 0.5f, 0.5f, 0.0f, 1.0f },
    { 0.6f, 0.6f, 0.5f, 1.0f },
    32.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::BlackRubber = {
    { 0.01f, 0.01f, 0.01f, 1.0f },
    { 0.4f,  0.4f,  0.4f,  1.0f },
    10.0f,
    { 0.02f, 0.02f, 0.02f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::CyanRubber = {
    { 0.4f,  0.5f, 0.5f, 1.0f },
    { 0.04f, 0.7f, 0.7f, 1.0f },
    10.0f,
    { 0.0f, 0.05f, 0.05f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::GreenRubber = {
    { 0.4f,  0.5f, 0.4f, 1.0f },
    { 0.04f, 0.7f, 0.04f, 1.0f },
    10.0f,
    { 0.0f, 0.05f, 0.0f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::RedRubber = {
    { 0.5f, 0.4f, 0.4f, 1.0f },
    { 0.7f, 0.04f, 0.04f, 1.0f },
    10.0f,
    { 0.05f, 0.0f, 0.0f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::WhiteRubber = {
    { 0.5f, 0.5f, 0.5f, 1.0f },
    { 0.7f, 0.7f, 0.7f, 1.0f },
    10.0f,
    { 0.05f, 0.05f, 0.05f, 1.0f }
};

const Cyrex::MaterialProperties Cyrex::Material::YellowRubber = {
    { 0.5f, 0.5f, 0.4f, 1.0f },
    { 0.7f, 0.7f, 0.04f, 1.0f },
    10.0f,
    { 0.05f, 0.05f, 0.0f, 1.0f }
};
