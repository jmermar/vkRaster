#include "types.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
namespace vkr {
glm::mat4 TransformData::getTransform() const {
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1), scale);

    glm::mat4 rotationMatrix =
        glm::rotate(glm::mat4(1), glm::degrees(rotation.y), glm::vec3(0, 1, 0));
    rotationMatrix = glm::rotate(glm::mat4(1), glm::degrees(rotation.x),
                                 glm::vec3(1, 0, 0)) *
                     rotationMatrix;
    rotationMatrix = glm::rotate(glm::mat4(1), glm::degrees(rotation.z),
                                 glm::vec3(0, 0, 1)) *
                     rotationMatrix;

    glm::mat4 translate = glm::translate(glm::mat4(1), position);
    return translate * rotationMatrix * scaleMatrix;
}
void CameraData::rotateX(float degrees) {
    auto angle = glm::radians(degrees / 2.f);
    glm::quat rotation(glm::cos(angle), glm::vec3(0, 1, 0) * glm::sin(angle));
    glm::quat rotationC = glm::conjugate(rotation);

    target = rotation * target * rotationC;
}
void CameraData::rotateY(float degrees) {
    auto angle = glm::radians(degrees / 2.f);
    glm::quat rotation(glm::cos(angle), glm::cross(target, glm::vec3(0, 1, 0)) *
                                            glm::sin(angle));
    glm::quat rotationC = glm::conjugate(rotation);

    target = rotation * target * rotationC;
}
glm::mat4 CameraData::getView() {
    return glm::lookAt(position, position + target, glm::vec3(0, 1, 0));
}
glm::mat4 CameraData::getProj() {
    return glm::perspective(fov, w / h, znear, zfar);
}
}  // namespace vkr