#define JSON_DIAGNOSTICS 1
#include "modules/Starter.hpp"
#include "modules/TextMaker.hpp"

std::vector<SingleText> outText = {
    {2, {"Playground Scene", "Press SPACE to exit"}, 0, 0}
};

struct Asset {
    const std::string ObjPath;
    const std::string TexturePath;
    const glm::vec3 pos;
    const float scale;
    const glm::vec3 rot;
};

// The uniform buffer object used in this example
struct UniformBufferObject {
    alignas(16) glm::mat4 mvpMat;
    alignas(16) glm::mat4 mMat;
    alignas(16) glm::mat4 nMat;
    alignas(16) glm::vec4 color;
};

struct GlobalUniformBufferObject {
    alignas(16) glm::vec3 lightDir;
    alignas(16) glm::vec4 lightColor;
    alignas(16) glm::vec3 eyePos;
    alignas(16) glm::vec4 eyeDir;
};

// The vertices data structures
struct Vertex {
    glm::vec3 pos;
    glm::vec2 UV;
    glm::vec3 norm;
};

const std::vector<Asset> AssetVector = {
    {"models/LargePlane.obj", "textures/water.png", {0.0f, 0.0f, 0.0f}, 1.0f, {0.0f, 0.0f, 0.0f}},
    {"models/Warships/10619_Battleship.obj", "textures/metal.jpeg", {0.0f, 0.0f, 0.0f}, 1.0f, {0.0f, 0.0f, 0.0f}}
};

const int numAssets = AssetVector.size();

const std::string vert = "shaders/PhongVert.spv";
const std::string frag = "shaders/PhongFrag.spv";

class FinalProject : public BaseProject {
protected:
    // Descriptor Layouts ["classes" of what will be passed to the shaders]
    DescriptorSetLayout DSLPlane;
    DescriptorSetLayout DSLBattleship;

    // Vertex formats
    VertexDescriptor VDPlane;
    VertexDescriptor VDBattleship;

    // Pipelines [Shader couples]
    Pipeline PPlane;
    Pipeline PBattleship;

    // Models
    Model largePlaneModel;
    Model battleshipModel;

    // Descriptor sets
    DescriptorSet DSPlane;
    DescriptorSet DSBattleship;

    // Textures
    Texture planeTexture;
    Texture battleshipTexture;

    // Application parameters
    glm::mat4 Wm_Plane;
    glm::mat4 Wm_Battleship;

    glm::vec4 MCol_Plane;
    glm::vec4 MCol_Battleship;

    TextMaker txt;

    // Camera parameters
    glm::vec3 CamPos = glm::vec3(0.0, 1.5, 7.0);
    float CamAlpha = 0.0f;
    float CamBeta = 0.0f;
    float Ar;
    bool WireFrame = false;

    void setWindowParameters() {
        windowWidth = 800;
        windowHeight = 600;
        windowTitle = "FinalProject - BattleShip";
        windowResizable = GLFW_TRUE;
        initialBackgroundColor = {0.0f, 0.0f, 0.0f, 1.0f};

        uniformBlocksInPool = 16 + 1;
        texturesInPool = 16;
        setsInPool = 16 + 1;

        Ar = 4.0f / 3.0f;
    }

    void onWindowResize(int w, int h) {
        std::cout << "Window resized to: " << w << " x " << h << "\n";
        Ar = (float)w / (float)h;
    }

    void localInit() {
        // Descriptor Layouts [what will be passed to the shaders]
        DSLPlane.init(this, {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
            {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},
            {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}
        });

        DSLBattleship.init(this, {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
            {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},
            {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}
        });

        // Vertex descriptors
        VDPlane.init(this, {
                {0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}
            }, {
                {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos),
                    sizeof(glm::vec3), POSITION},
                {0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, UV),
                    sizeof(glm::vec2), UV},
                {0, 2, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, norm),
                    sizeof(glm::vec3), NORMAL}
            });

        VDBattleship.init(this, {
                {0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}
            }, {
                {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos),
                    sizeof(glm::vec3), POSITION},
                {0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, UV),
                    sizeof(glm::vec2), UV},
                {0, 2, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, norm),
                    sizeof(glm::vec3), NORMAL}
            });

        // Pipelines [Shader couples]
        PPlane.init(this, &VDPlane, "shaders/PhongVert.spv", "shaders/PhongFrag.spv", {&DSLPlane});
        PPlane.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
                                VK_CULL_MODE_BACK_BIT, false);

        PBattleship.init(this, &VDBattleship, "shaders/PhongVert.spv", "shaders/PhongFrag.spv", {&DSLBattleship});
        PBattleship.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
                                VK_CULL_MODE_BACK_BIT, false);

        // Models, textures and Descriptors (values assigned to the uniforms)
        largePlaneModel.init(this, &VDPlane, "models/Water.obj", OBJ);
        battleshipModel.init(this, &VDBattleship, "models/Warships/Battleship.obj", OBJ);

        // Initialize the world matrix and color for the large plane model
        Wm_Plane = glm::mat4(1.0f);
        MCol_Plane = glm::vec4(1.0f);

        // Initialize the world matrix and color for the battleship model
        Wm_Battleship = glm::mat4(1.0f);
        MCol_Battleship = glm::vec4(1.0f);

        // Create the textures
        planeTexture.init(this, "textures/water.png");
        battleshipTexture.init(this, "textures/metal.jpeg");

        // Initialize text maker
        txt.init(this, &outText);
    }

    void pipelinesAndDescriptorSetsInit() {
        // Create pipeline
        PPlane.create();
        PBattleship.create();

        // Define the data set
        DSPlane.init(this, &DSLPlane, { 
            {0, UNIFORM, sizeof(UniformBufferObject), nullptr}, 
            {1, TEXTURE, 0, &planeTexture},
            {2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
        });

        DSBattleship.init(this, &DSLBattleship, { 
            {0, UNIFORM, sizeof(UniformBufferObject), nullptr}, 
            {1, TEXTURE, 0, &battleshipTexture},
            {2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
        });

        txt.pipelinesAndDescriptorSetsInit();
    }

    void pipelinesAndDescriptorSetsCleanup() {
        // Cleanup pipeline
        PPlane.cleanup();
        PBattleship.cleanup();

        // Cleanup descriptor set
        DSPlane.cleanup();
        DSBattleship.cleanup();

        txt.pipelinesAndDescriptorSetsCleanup();
    }

    void localCleanup() {
        // Cleanup texture
        planeTexture.cleanup();
        battleshipTexture.cleanup();

        // Cleanup model
        largePlaneModel.cleanup();
        battleshipModel.cleanup();

        // Cleanup descriptor set layout
        DSLPlane.cleanup();
        DSLBattleship.cleanup();

        // Destroy pipeline
        PPlane.destroy();
        PBattleship.destroy();

        txt.localCleanup();
    }

    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
        // Bind the pipeline for the plane
        PPlane.bind(commandBuffer);
        // Bind the model for the plane
        largePlaneModel.bind(commandBuffer);
        // Bind the descriptor set for the plane
        DSPlane.bind(commandBuffer, PPlane, 0, currentImage);
        // Draw the plane
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(largePlaneModel.indices.size()), 1, 0, 0, 0);

        // Bind the pipeline for the battleship
        PBattleship.bind(commandBuffer);
        // Bind the model for the battleship
        battleshipModel.bind(commandBuffer);
        // Bind the descriptor set for the battleship
        DSBattleship.bind(commandBuffer, PBattleship, 0, currentImage);
        // Draw the battleship
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(battleshipModel.indices.size()), 1, 0, 0, 0);

        txt.populateCommandBuffer(commandBuffer, currentImage, 0);
    }

    void updateUniformBuffer(uint32_t currentImage) {
        float deltaT;
        glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
        bool fire = false;
        getSixAxis(deltaT, m, r, fire);

        const float ROT_SPEED = glm::radians(120.0f);
        const float MOVE_SPEED = 2.0f;

        CamAlpha = CamAlpha - ROT_SPEED * deltaT * r.y;
        CamBeta  = CamBeta  - ROT_SPEED * deltaT * r.x;
        CamBeta  =  CamBeta < glm::radians(-90.0f) ? glm::radians(-90.0f) :
                   (CamBeta > glm::radians( 90.0f) ? glm::radians( 90.0f) : CamBeta);

        glm::vec3 ux = glm::rotate(glm::mat4(1.0f), CamAlpha, glm::vec3(0,1,0)) * glm::vec4(1,0,0,1);
        glm::vec3 uz = glm::rotate(glm::mat4(1.0f), CamAlpha, glm::vec3(0,1,0)) * glm::vec4(0,0,1,1);
        CamPos = CamPos + MOVE_SPEED * m.x * ux * deltaT;
        CamPos = CamPos + MOVE_SPEED * m.y * glm::vec3(0,1,0) * deltaT;
        CamPos = CamPos + MOVE_SPEED * m.z * uz * deltaT;

        if(glfwGetKey(window, GLFW_KEY_SPACE)) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }

        // Here is where you actually update your uniforms
        glm::mat4 M = glm::perspective(glm::radians(45.0f), Ar, 0.1f, 50.0f);
        M[1][1] *= -1;

        glm::mat4 Mv =  glm::rotate(glm::mat4(1.0), -CamBeta, glm::vec3(1,0,0)) *
                        glm::rotate(glm::mat4(1.0), -CamAlpha, glm::vec3(0,1,0)) *
                        glm::translate(glm::mat4(1.0), -CamPos);

        glm::mat4 ViewPrj =  M * Mv;
        UniformBufferObject ubo{};

        // Update uniforms for the plane
        ubo.mMat = Wm_Plane;
        ubo.mvpMat = ViewPrj * ubo.mMat;
        ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
        ubo.color = MCol_Plane;

        // Update global uniforms
        GlobalUniformBufferObject gubo{};
        gubo.lightDir = glm::vec3(cos(glm::radians(135.0f)), sin(glm::radians(135.0f)), 0.0f);
        gubo.lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        gubo.eyePos = CamPos;

        // Map the uniforms for the plane
        DSPlane.map(currentImage, &ubo, sizeof(ubo), 0);
        DSPlane.map(currentImage, &gubo, sizeof(gubo), 2);

        // Update uniforms for the battleship
		ubo.mMat = glm::mat4(1.0f);
		ubo.mvpMat = ViewPrj * ubo.mMat;
		ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
		ubo.color = MCol_Battleship;

        // Map the uniforms for the battleship
        DSBattleship.map(currentImage, &ubo, sizeof(ubo), 0);
        DSBattleship.map(currentImage, &gubo, sizeof(gubo), 2);
    }
};

// This is the main: probably you do not need to touch this!
int main() {
    FinalProject app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
