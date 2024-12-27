#pragma once

namespace MFA {
class IShadingPipeline {
public:
    virtual ~IShadingPipeline() = default;
    virtual void Reload() = 0;
};
}
