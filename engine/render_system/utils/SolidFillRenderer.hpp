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
			Position const & pos0, 
			Position const & pos1, 
			Position const & pos2, 
			Position const & pos3, 
			
			Color const & color0, 
			Color const & color1, 
			Color const & color2, 
			Color const & color3,

			float borderRadius
		);

		void Draw(RT::CommandRecordState& recordState, LocalBufferTracker const& vertexBuffer) const;

	private:

		std::shared_ptr<Pipeline> _pipeline;

	};
}
