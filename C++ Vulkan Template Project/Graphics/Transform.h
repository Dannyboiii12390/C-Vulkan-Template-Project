#pragma once

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace Engine {

	struct Transform {

		glm::vec3 position{ 0.0f, 0.0f, 0.0f };
		glm::vec3 rotation{ 0.0f, 0.0f, 0.0f };
		glm::vec3 scale{ 1.0f, 1.0f, 1.0f };

		glm::mat4 getMatrix() const
		{
			const glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);
			const glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
			const glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
			const glm::mat4 rotationZ = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
			const glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scale);
			const glm::mat4 rotationMat = rotationZ * rotationY * rotationX;
			return translation * rotationMat * scaleMat;
		};
	};
}