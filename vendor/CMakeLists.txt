include(FetchContent)
FetchContent_Declare(
  sdl3
  GIT_REPOSITORY      https://github.com/libsdl-org/SDL.git
  GIT_TAG release-2.30.1
)

FetchContent_Declare(
  vk-bootstrap
  GIT_REPOSITORY      https://github.com/charles-lunarg/vk-bootstrap.git
  GIT_TAG v1.3.280
)

FetchContent_MakeAvailable(sdl3)
FetchContent_MakeAvailable(vk-bootstrap)