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

			float topLeftBorderRadius,
			float bottomLeftBorderRadius,
			float topRightBorderRadius,
			float bottomRightBorderRadius
		);

		void Draw(RT::CommandRecordState& recordState, LocalBufferTracker const& vertexBuffer) const;

	private:

		std::shared_ptr<Pipeline> _pipeline;

	};
}
