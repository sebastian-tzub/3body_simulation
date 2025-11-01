#include "simulation.h"
#include <fstream>
#include <iomanip>

float GRAVITATIONAL_CONSTANT = 6.67430 * pow(10, -11); 
bool endSimulation = false; 

// random number generator 
default_random_engine generator(random_device{}()); 

double lowerBoundMass = 0; 
double upperBoundMass = 1; 

double lowerBoundRadius = 0; 
double upperBoundRadius = 1; 

double lowerBoundPosition = 0; 
double upperBoundPosition = 1; 

double lowerBoundVelocity = 0; 
double upperBoundVelocity = 1; 

// range distributor for initial values of mass, position and velocity of planet 
uniform_real_distribution<float> massDistribution(lowerBoundMass, upperBoundMass);
uniform_real_distribution<float> radiusDistribution(lowerBoundRadius, upperBoundRadius);
uniform_real_distribution<float> positionDistribution(lowerBoundPosition, upperBoundPosition);
uniform_real_distribution<float> velocityDistribution(lowerBoundVelocity, upperBoundVelocity);

float dt = 1.0f;

// Logging variables
int logStepCount = 0;
int maxLogSteps = 100; // number of timesteps to log
bool isLogging = true;
ofstream simLog;

class Planet {
public:
    float mass;
    float radius;

    glm::vec3 position;
    glm::vec3 velocity;

    Planet() {
        mass = massDistribution(generator);
        radius = radiusDistribution(generator);

        position = glm::vec3 (
            positionDistribution(generator),
            positionDistribution(generator),
            positionDistribution(generator));

        velocity = glm::vec3 (
            velocityDistribution(generator),
            velocityDistribution(generator),
            velocityDistribution(generator));
    };
};

// array storing created planets
vector<Planet> planets;

bool collisionChecker(Planet planet1, Planet planet2);

vector<glm::vec3> summation (){

    vector<glm::vec3> accelerations(planets.size());

    for (int i = 0; i < planets.size(); i ++) {
        glm::vec3 currentAccel(0,0,0);
        for (int j = 0; j < planets.size(); j++) {
            if (i==j) {
                continue;
            } else {
                if (collisionChecker(planets[i], planets[j])) {
                    endSimulation = true;
                } else {
                    glm::vec3 vector = planets[j].position - planets[i].position;
                    float vector_magnitude = glm::length(vector);
                    currentAccel += GRAVITATIONAL_CONSTANT * planets[j].mass * vector / float(pow(vector_magnitude,3));
                }
            }
        }
        accelerations[i] = currentAccel;
    }
    return accelerations;
}

void solver () {
    vector<glm::vec3> accelerations = summation();
    for (int i = 0; i < planets.size(); i++) {
        planets[i].velocity += accelerations[i] * dt;
        planets[i].position += planets[i].velocity * dt;
    }
}

bool collisionChecker(Planet planet1, Planet planet2){

    double distance = glm::length(planet1.position - planet2.position);

    // get sum of radisu of planets
    double radiusSum = planet1.radius + planet2.radius;

    // if distance < sum return true
    return distance < radiusSum;
}

void populate (vector<Planet>& vector, int num) {
    int i = 0;
    while (i < num) {
        Planet planet;
        vector.push_back(planet);
        i++;
    }
}

void logSimulationState(int timestep) {
    if (!isLogging || logStepCount >= maxLogSteps) {
        if (isLogging && logStepCount == maxLogSteps) {
            simLog.close();
            isLogging = false;
            cout << "Logging completed. Saved " << maxLogSteps << " timesteps." << endl;
        }
        return;
    }

    if (logStepCount == 0) {
        // Initialize log file with header
        simLog.open("simulation_log.txt");
        simLog << "SIMULATION LOG" << endl;
        simLog << "==============" << endl;
        simLog << "Gravitational Constant: " << GRAVITATIONAL_CONSTANT << endl;
        simLog << "Timestep (dt): " << dt << endl;
        simLog << "Number of Planets: " << planets.size() << endl;
        simLog << endl;
    }

    // Log current timestep data
    simLog << "TIMESTEP " << timestep << endl;
    for (size_t i = 0; i < planets.size(); i++) {
        simLog << "  Planet " << i << ":" << endl;
        simLog << "    mass: " << planets[i].mass << ", radius: " << planets[i].radius << endl;
        simLog << "    pos: (" << planets[i].position.x << ", "
               << planets[i].position.y << ", " << planets[i].position.z << ")" << endl;
        simLog << "    vel: (" << planets[i].velocity.x << ", "
               << planets[i].velocity.y << ", " << planets[i].velocity.z << ")" << endl;
    }
    simLog << endl;

    logStepCount++;
}

int main (){

    // GLFW Initialization
    if (!glfwInit()) {
        cerr << "Failed to initialize GLFW" << endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(650, 650, "3-Body Simulation", NULL, NULL);

    // error check
    if (window == NULL) {
        cerr << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // load GLAD - must be after making context current!
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cerr << "Failed to initialize GLAD" << endl;
        glfwTerminate();
        return -1;
    }

    // viewport of openGL in window
    glViewport(0, 0, 650, 650);

    // PLANET SIMULATION INITIALIZATION
    populate(planets, 3);

    cout << "Simulation started with " << planets.size() << " planets" << endl;
    cout << "Logging first " << maxLogSteps << " timesteps to simulation_log.txt" << endl;

    int timestep = 0;

    // COMBINED SIMULATION AND RENDERING LOOP
    while (!glfwWindowShouldClose(window) && !endSimulation) {
        // Poll for window events
        glfwPollEvents();

        // Run physics simulation step
        solver();

        // Log simulation state
        logSimulationState(timestep);

        // Clear the screen
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // TODO: Add rendering code here to visualize planets

        // Swap buffers
        glfwSwapBuffers(window);

        // Control simulation speed
        std::this_thread::sleep_for(std::chrono::milliseconds((int)dt));

        timestep++;
    }

    // Ensure log is properly closed if simulation ends early
    if (isLogging && simLog.is_open()) {
        simLog.close();
        cout << "Simulation ended at timestep " << timestep << endl;
    }

    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
