#include "vk_mesh.h"

VertexInputDescription Vertex::get_vertex_description() {
	VertexInputDescription description;

	//we will have just 1 vertex buffer binding, with a per-vertex rate
	vk::VertexInputBindingDescription main_binding = {};
	main_binding.binding = 0;
	main_binding.stride = sizeof(Vertex);
	main_binding.inputRate = vk::VertexInputRate::eVertex;

	description.bindings.push_back(main_binding);

	//Position will be stored at Location 0
	vk::VertexInputAttributeDescription position_attribute = {};
	position_attribute.binding = 0;
	position_attribute.location = 0;
	position_attribute.format = vk::Format::eR32G32B32A32Sfloat;
	position_attribute.offset = offsetof(Vertex, position);

	//Normal will be stored at Location 1
	vk::VertexInputAttributeDescription normal_attribute = {};
	normal_attribute.binding = 0;
	normal_attribute.location = 1;
	normal_attribute.format = vk::Format::eR32G32B32A32Sfloat;
	normal_attribute.offset = offsetof(Vertex, normal);

	//Color will be stored at Location 2
	vk::VertexInputAttributeDescription color_attribute = {};
	color_attribute.binding = 0;
	color_attribute.location = 2;
	color_attribute.format = vk::Format::eR32G32B32A32Sfloat;
	color_attribute.offset = offsetof(Vertex, color);

	description.attributes.push_back(position_attribute);
	description.attributes.push_back(normal_attribute);
	description.attributes.push_back(color_attribute);
	return description;
}
