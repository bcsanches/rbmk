#define SDL_MAIN_USE_CALLBACKS

#include <algorithm>
#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "Math/rbVec3.h"

const int WIDTH = 640;
const int HEIGHT = 480;

static SDL_Window *window = nullptr;
static SDL_Renderer *renderer = nullptr;
static SDL_Texture *texture = nullptr;

static Uint32 pixels[WIDTH * HEIGHT];

struct Ray
{
	rbmk::Math::Vec3 origin;
	rbmk::Math::Vec3 dir;
};

struct Sphere
{
	rbmk::Math::Vec3 center;
	float radius;

	rbmk::Math::Vec3 color;
};

struct Plane
{
	rbmk::Math::Vec3 normal;
	float d;
};

struct Brush
{
	std::vector<Plane> planes;
	rbmk::Math::Vec3 color{ 1,1,1 };
};

struct Camera
{
	rbmk::Math::Vec3 position;

	float yaw;
	float pitch;

	float fov;
};

struct Light
{
	rbmk::Math::Vec3 position;
	rbmk::Math::Vec3 color;
	float intensity;
};

struct Hit
{
	float t;
	rbmk::Math::Vec3 normal;
	rbmk::Math::Vec3 point;
};

inline bool intersectSphere(const Ray &ray, const Sphere &sphere, float &t) noexcept
{
	const auto oc{ ray.origin - sphere.center };

	float b = oc.Dot(ray.dir);
	float c = oc.Dot(oc) - sphere.radius * sphere.radius;

	float discriminant = b * b - c;

	if (discriminant < 0)
		return false;

	float sqrtD = sqrt(discriminant);

	float t0 = -b - sqrtD;
	float t1 = -b + sqrtD;

	t = (t0 > 0) ? t0 : t1;

	return t > 0;
}

bool intersectPlane(const Ray &ray, const Plane &plane, float &t)
{
	float denom = ray.dir.Dot(plane.normal);

	if (fabs(denom) < 1e-6)
		return false;

	t = -(ray.origin.Dot(plane.normal) + plane.d) / denom;

	return t >= 0;
}

bool intersectBrush(
	const Ray &ray,
	const Brush &brush,
	Hit &hit)
{
	float tEnter = 0.0f;
	float tExit = 1e30f;

	const Plane *enterPlane = nullptr;

	for (const Plane &plane : brush.planes)
	{
		float dist = ray.origin.Dot(plane.normal) + plane.d;
		float denom = ray.dir.Dot(plane.normal);

		if (fabs(denom) < 1e-6)
		{
			if (dist > 0)
				return false;

			continue;
		}

		float t = -dist / denom;

		if (denom < 0)
		{
			if (t > tEnter)
			{
				tEnter = t;
				enterPlane = &plane;
			}
		}
		else
		{
			if (t < tExit)
				tExit = t;
		}

		if (tEnter > tExit)
			return false;
	}

	if (!enterPlane)
		return false;

	hit.t = tEnter;
	hit.normal = enterPlane->normal;
	hit.point = ray.origin + ray.dir * tEnter;

	return true;
}

Uint32 packColor(int r, int g, int b)
{
	return (255 << 24) | (r << 16) | (g << 8) | b;
}

Uint32 packColor(const rbmk::Math::Vec3 &color)
{
	int r = std::min(255, (int)(color.x * 255));
	int g = std::min(255, (int)(color.y * 255));
	int b = std::min(255, (int)(color.z * 255));
	
	return packColor(r, g, b);
}

class JobSystem
{
public:

	JobSystem(size_t threads = std::thread::hardware_concurrency())
		: stop(false), activeJobs(0)
	{
		for (size_t i = 0; i < threads; i++)
		{
			workers.emplace_back([this]()
				{
					Worker();
				});
		}
	}

	~JobSystem()
	{
		stop = true;
		condition.notify_all();

		for (auto &t : workers)
			t.join();
	}

	void Submit(std::function<void()> job)
	{
		activeJobs++;

		{
			std::unique_lock<std::mutex> lock(queueMutex);
			jobs.push(job);
		}

		condition.notify_one();
	}

	void Wait()
	{
		std::unique_lock<std::mutex> lock(waitMutex);

		waitCondition.wait(lock, [this]()
			{
				return activeJobs.load() == 0;
			});
	}

private:

	void Worker()
	{
		while (true)
		{
			std::function<void()> job;

			{
				std::unique_lock<std::mutex> lock(queueMutex);

				condition.wait(lock, [this]()
					{
						return stop || !jobs.empty();
					});

				if (stop && jobs.empty())
					return;

				job = jobs.front();
				jobs.pop();
			}

			job();

			if (--activeJobs == 0)
			{
				waitCondition.notify_all();
			}
		}
	}

private:

	std::vector<std::thread> workers;
	std::queue<std::function<void()>> jobs;

	std::mutex queueMutex;
	std::condition_variable condition;

	std::atomic<int> activeJobs;

	std::mutex waitMutex;
	std::condition_variable waitCondition;

	std::atomic<bool> stop;
};

static Camera camera;
static std::vector<Light> lights =
{
	{{-5,5,-2},{1,1,1},1.0f},   // luz branca
	{{5,3,-3},{1,0,0},0.7f},    // luz vermelha
	{{0,6,-5},{0,0,1},0.7f}     // luz azul
};

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv)
{
	if (!SDL_Init(SDL_INIT_VIDEO)) 
	{
		SDL_Log("SDL_Init(SDL_INIT_VIDEO) failed: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	if (!SDL_CreateWindowAndRenderer("RBMK", WIDTH, HEIGHT, 0, &window, &renderer))
	{
		SDL_Log("SDL_CreateWindowAndRenderer() failed: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	texture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		WIDTH, 
		HEIGHT
	);

	if(!texture)
	{
		SDL_Log("SDL_CreateTexture() failed: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	SDL_SetWindowRelativeMouseMode(window, true);

	camera.position = { 0,0,2 };
	camera.yaw = 3.14159f;
	camera.pitch = 0;
	camera.fov = 90.0f * 3.14159f / 180.0f;

	return SDL_APP_CONTINUE;
}

static float moveFwd = 0;
static float moveStrafe = 0;
static float rotateYaw = 0;
static float rotatePitch = 0;

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
	switch (event->type)
	{
		case SDL_EVENT_QUIT:
			return SDL_APP_SUCCESS;	

		case SDL_EVENT_KEY_DOWN:  /* quit if user hits ESC key */
			if (event->key.scancode == SDL_SCANCODE_ESCAPE) 
			{
				return SDL_APP_SUCCESS;
			}

			if(event->key.scancode == SDL_SCANCODE_W)
				moveFwd = 1;
			else if(event->key.scancode == SDL_SCANCODE_S)
				moveFwd = -1;
			else if(event->key.scancode == SDL_SCANCODE_A)
				moveStrafe = -1;
			else if(event->key.scancode == SDL_SCANCODE_D)
				moveStrafe = 1;
			break;

		case SDL_EVENT_KEY_UP:
			if(event->key.scancode == SDL_SCANCODE_W || event->key.scancode == SDL_SCANCODE_S)
				moveFwd = 0;
			else if(event->key.scancode == SDL_SCANCODE_A || event->key.scancode == SDL_SCANCODE_D)
				moveStrafe = 0;
			break;

		case SDL_EVENT_MOUSE_MOTION: 
			rotateYaw = event->motion.x;
			rotatePitch = event->motion.y;			
			break;
	}

	return SDL_APP_CONTINUE;
}

uint64_t startTime = 0;
uint64_t frameCount = 0;

static JobSystem g_jobSystem;

SDL_AppResult SDL_AppIterate(void *appstate)
{
	constexpr auto NUM_SPHERES = 6;
	static Sphere spheres[NUM_SPHERES];

	spheres[0] = { { -2.5f,0,-3 }, 1, {1, 0, 0} };
	spheres[1] = { { 0,0,-3 }, 1, {0, 1, 0} };
	spheres[2] = { { 2.5f,0,-3 }, 1, {0, 0, 1} };
	spheres[3] = { { 4.5f,0,-3 }, 1.2f, {1, 0, 1} };
	spheres[4] = { { 6.5f,0,-3 }, 1.2f, {1, 1, 1} };
	spheres[5] = { { -0.5f,1.5f,2 }, 0.5f, {1, 1, 1} };

	Brush cube;

	cube.planes = {
		{{ 1,0,0},-1},
		{{-1,0,0},-1},
		{{0, 1,0},-1},
		{{0,-1,0},-1},
		{{0,0, 1},-3},
		{{0,0,-1}, 1}
	};		

	rbmk::Math::Vec3 camFwd =
	{
		cos(camera.pitch) * sin(camera.yaw),
		sin(camera.pitch),
		cos(camera.pitch) * cos(camera.yaw)
	};

	camFwd.Normalize();	
	rbmk::Math::Vec3 worldUp{ 0,1,0 };

	auto camRight = rbmk::Math::Vec3::Cross(camFwd, worldUp);
	camRight.Normalize();

	auto camUp = rbmk::Math::Vec3::Cross(camRight, camFwd);

	const float speed = 0.05f;
	camera.position += camFwd * moveFwd * speed;
	camera.position += camRight * moveStrafe * speed;	

	float mx, my;
	SDL_GetRelativeMouseState(&mx, &my);

	float sensitivity = 0.002f;

	camera.yaw -= mx * sensitivity;
	camera.pitch += -my * sensitivity;

	rotateYaw = rotatePitch = 0;

	float scale = tan(camera.fov * 0.5f);

	rbmk::Math::Vec3 light{ 1,1,-1 };
	light.Normalize();

#if 1
	for (int y = 0; y < HEIGHT; y++)
	{
		g_jobSystem.Submit([y, light,  cube, camFwd, camRight, camUp, scale]()
			{
				for (int x = 0; x < WIDTH; x++)
				{					
					float px = (2 * (x + 0.5f) / WIDTH - 1) * scale * (WIDTH / (float)HEIGHT);
					float py = (1 - 2 * (y + 0.5f) / HEIGHT) * scale;

					auto rayDir = camFwd + camRight * px + camUp * py;

					Ray ray;
					ray.origin = camera.position;
					ray.dir = rayDir;
					ray.dir.Normalize();

					float closestT = 1e30f;
					Sphere *hitSphere = nullptr;

					pixels[y * WIDTH + x] = packColor(30, 30, 50);
					for (auto i = 0; i < NUM_SPHERES; i++)
					{
						float t;
						if (intersectSphere(ray, spheres[i], t))
						{
							if (t < closestT)
							{
								closestT = t;
								hitSphere = &spheres[i];
							}
						}
					}

					if (hitSphere)
					{			
						auto hit = ray.origin + ray.dir * closestT;
						auto normal = hit - hitSphere->center;
						normal.Normalize();

						rbmk::Math::Vec3 finalColor;
						finalColor.Zero();

						for (const auto &light : lights)
						{
							auto lightDir = light.position - hit;
							float lightDist = lightDir.Length();

							//normalize...
							lightDir *= 1.0f / lightDist;							

							Ray shadowRay;

							shadowRay.origin = hit + normal * 0.001f;
							shadowRay.dir = lightDir;

							bool inShadow = false;

							for (auto &s : spheres)
							{
								if (&s == hitSphere)
									continue;

								float tShadow;

								if (intersectSphere(shadowRay, s, tShadow))
								{
									if (tShadow < lightDist)
									{
										inShadow = true;
										break;
									}
								}
							}

							if (!inShadow)
							{
								Hit shadowHit;
								if (intersectBrush(shadowRay, cube, shadowHit))
								{
									inShadow = true;
								}
							}

							if (!inShadow)
							{
								float diffuse = std::max(0.0f, normal.Dot(lightDir));

								auto lightContribution = hitSphere->color * light.color * diffuse * light.intensity;

								finalColor += lightContribution;
							}
						}						

						pixels[y * WIDTH + x] = packColor(finalColor);
					}

					Hit hit;

					if (intersectBrush(ray, cube, hit))
					{
						if (hit.t < closestT)
						{
							closestT = hit.t;

							rbmk::Math::Vec3 finalColor;
							finalColor.Zero();

							for (const auto &light : lights)
							{
								auto lightDir = light.position - hit.point;
								float lightDist = lightDir.Length();

								//normalize...
								lightDir *= 1.0f / lightDist;

								Ray shadowRay;

								shadowRay.origin = hit.point + hit.normal * 0.001f;
								shadowRay.dir = lightDir;

								bool inShadow = false;

								for (auto &s : spheres)
								{
									if (&s == hitSphere)
										continue;

									float tShadow;

									if (intersectSphere(shadowRay, s, tShadow))
									{
										if (tShadow < lightDist)
										{
											inShadow = true;
											break;
										}
									}
								}								

								if (!inShadow)
								{
									float diffuse = std::max(0.0f, hit.normal.Dot(lightDir));

									auto contribution =
										cube.color *
										light.color *
										diffuse *
										light.intensity;

									finalColor += contribution;
								}								
							}
							
							pixels[y * WIDTH + x] = packColor(finalColor);
						}

						//Vec3 hit = ray.origin + ray.dir * tBrush;
					}
				}
			}
		);		
	}
#endif

	g_jobSystem.Wait();

	SDL_UpdateTexture(texture, nullptr, pixels, WIDTH * sizeof(Uint32));

	/* you have to draw the whole window every frame. Clearing it makes sure the whole thing is sane. */
	SDL_RenderClear(renderer);  /* clear whole window to that fade color. */

	SDL_RenderTexture(renderer, texture, nullptr, nullptr);
	SDL_RenderPresent(renderer);

	++frameCount;

	auto currentTime = SDL_GetTicks();
	if (currentTime - startTime >= 1000)
	{
		startTime = currentTime;
		SDL_Log("FPS: %llu", frameCount);

		char str[256];
		sprintf(str, "RBMK - FPS: %llu", frameCount);

		SDL_SetWindowTitle(window, str);

		frameCount = 0;
	}

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);	
}
