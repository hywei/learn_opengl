#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera
{
public:
	static const glm::vec3 WORLD_UP_DIR;

	Camera() { }
	Camera(const glm::vec3& camera_pos, const glm::vec3& target_pos) :
		camera_position_(camera_pos), target_position_(target_pos) {}

	void setPosition(const glm::vec3& pos) { camera_position_ = pos; }
	void setTarget(const glm::vec3& target_pos) { target_position_ = target_pos; }


	glm::vec3 getPosition() const { return camera_position_; }
	glm::vec3 getDirection() const { return glm::normalize(target_position_ - camera_position_); }
	glm::mat4 getLookAt() const { return glm::lookAt(camera_position_, target_position_, WORLD_UP_DIR); }


protected:
	glm::vec3 camera_position_{ 0.f, 0.f, 0.f };
	glm::vec3 target_position_{ 0.f, 0.f, 0.f };

};

const glm::vec3 Camera::WORLD_UP_DIR = glm::vec3(0.f, 1.f, 0.f);
