#pragma once
#include <src/Core/Timestep.h>
#include <src/Rendering/Buffers/Framebuffer.h>
#include <src/UI/Nodes/Rect.h>
#include "yoga/Yoga.h"
#include "Nodes/Node.h"
#include "Nodes/Canvas.h"
#include "src/Core/Maths.h"
#include "Styling/Stylesheet.h"
#include "Font/Font.h"

namespace Nuake {
	namespace UI {
		class UserInterface
		{
		private:
			Ref<FrameBuffer> m_Framebuffer; // Texture of the interface.
			std::string m_Name;
			Ref<Canvas> Root;
			YGConfigRef yoga_config;
			YGNodeRef yoga_root;
		public:
			Ref<Font> font;
			const int Width = 1920;
			const int Height = 1080;

			UserInterface(const std::string& name);
			~UserInterface();

			void Reload();

			static Ref<UserInterface> New(const std::string& name);
			void Calculate(int available_width, int available_height);

			void CreateYogaLayout();
			void CreateYogaLayoutRecursive(Ref<Node> node, YGNodeRef yoga_node);
			void Draw(Vector2 size);
			void DrawRecursive(Ref<Node> node, float z);
			void Update(Timestep ts);

			void RecursiveMouseClick(Ref<Node> node, Vector2 pos);

			void ConsumeMouseClick(Vector2 pos);
		};
	}
}
