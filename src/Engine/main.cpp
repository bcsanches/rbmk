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

const int WIDTH = 800;
const int HEIGHT = 600;

static SDL_Window *window = nullptr;
static SDL_Renderer *renderer = nullptr;
static SDL_Texture *texture = nullptr;

static SDL_FRect mouseposrect;

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
};

struct Plane
{
	rbmk::Math::Vec3 normal;
	float d;
};

struct Brush
{
	std::vector<Plane> planes;
};

bool intersectSphere(const Ray &ray, const Sphere &sphere, float &t)
{
	auto oc = ray.origin - sphere.center;

	float a = ray.dir.Dot(ray.dir);
	float b = 2.0f * oc.Dot(ray.dir);
	float c = oc.Dot(oc) - sphere.radius * sphere.radius;

	float disc = b * b - 4 * a * c;

	if (disc < 0)
		return false;

	t = (-b - (float)sqrt(disc)) / (2.0f * a);
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
	float &tHit,
	rbmk::Math::Vec3 &normal)
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

	tHit = tEnter;
	normal = enterPlane->normal;

	return true;
}

struct Hit
{
	float t;
	rbmk::Math::Vec3 normal;
};

Uint32 packColor(int r, int g, int b)
{
	return (255 << 24) | (r << 16) | (g << 8) | b;
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

	return SDL_APP_CONTINUE;
}

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
			break;

		case SDL_EVENT_MOUSE_MOTION:  /* keep track of the latest mouse position */
			/* center the square where the mouse is */
			mouseposrect.x = event->motion.x - (mouseposrect.w / 2);
			mouseposrect.y = event->motion.y - (mouseposrect.h / 2);
			break;
	}

	return SDL_APP_CONTINUE;
}

uint64_t startTime = 0;
uint64_t frameCount = 0;

static JobSystem g_jobSystem;

SDL_AppResult SDL_AppIterate(void *appstate)
{
	constexpr auto NUM_SPHERES = 3;
	static Sphere spheres[NUM_SPHERES];

	spheres[0] = { { -1.5f,0,-3 }, 1 };
	spheres[1] = { { 0,0,-3 }, 1 };
	spheres[2] = { { 1.5f,0,-3 }, 1 };	

	rbmk::Math::Vec3 light{1,1,-1};
	light.Normalize();

	Brush cube;

	cube.planes = {
		{{ 1,0,0},-1},
		{{-1,0,0},-1},
		{{0, 1,0},-1},
		{{0,-1,0},-1},
		{{0,0, 1},-3},
		{{0,0,-1}, 1}
	};


#if 1
	for (int y = 0; y < HEIGHT; y++)
	{
		g_jobSystem.Submit([y, light, cube]()
			{
				for (int x = 0; x < WIDTH; x++)
				{
					float px = (2 * (x + 0.5f) / WIDTH - 1) * (WIDTH / (float)HEIGHT);
					float py = 1 - 2 * (y + 0.5f) / HEIGHT;

					Ray ray;
					ray.origin = { 0,0,0 };
					ray.dir = rbmk::Math::Vec3::Normalize(px, py, -1);

					float dist = 9999999999.0f;
					float t;

					pixels[y * WIDTH + x] = packColor(30, 30, 50);
					for (auto i = 0; i < NUM_SPHERES; i++)
					{
						if (intersectSphere(ray, spheres[i], t))
						{
							if (t >= dist)
								continue;

							dist = t;

							auto hit = ray.origin + ray.dir * t;
							auto normal = hit - spheres[i].center;
							normal.Normalize();

							float diffuse = std::max(0.0f, normal.Dot(light));

							int c = (int)(diffuse * 255);

							pixels[y * WIDTH + x] = packColor(c, c, c);

							break;
						}
					}

					Hit hit;

					if (intersectBrush(ray, cube, hit.t, hit.normal))
					{
						if (hit.t < dist)
						{
							dist = hit.t;
							float diffuse = std::max(0.0f, hit.normal.Dot(light));
							int c = (int)(diffuse * 255);
							pixels[y * WIDTH + x] = packColor(c, 0, 0);
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
