#pragma once
#include "BufferTracker.hpp"
#include "pipeline/SolidFillPipeline.hpp"

namespace MFA
{
	class SolidFillRenderer
	{
	public:

		using Pipeline = SolidFillPipeline;
		using Position = Pipeline::Position;
		using Color = Pipeline::Color;
		using Radius = Pipeline::Radius;

		explicit SolidFillRenderer(std::shared_ptr<Pipeline> pipeline);

		static std::shared_ptr<LocalBufferTracker> AllocateBuffer(
			Position const & topLeftPos,
			Position const & bottomLeftPos,
			Position const & topRightPos,
			Position const & bottomRightPos,
			
			Color const & topLeftColor,
			Color const & bottomLeftColor,
			Color const & topRightColor,
			Color const & bottomRightColor,

			Radius const & topLeftBorderRadius,
			Radius const & bottomLeftBorderRadius,
			Radius const & topRightBorderRadius,
			Radius const & bottomRightBorderRadius
		);

		void Draw(
            RT::CommandRecordState & recordState,
            Pipeline::PushConstants const & pushConstants,
            LocalBufferTracker const & vertexBuffer
        ) const;

	private:

		std::shared_ptr<Pipeline> _pipeline;

	};
}
