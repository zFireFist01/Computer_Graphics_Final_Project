// This has been adapted from the Vulkan tutorial

#include "modules/Starter.hpp"
#include "modules/TextMaker.hpp"


#define NPLANE 2

std::vector<SingleText> outText = {
    {2, {"PlayGround Scene", "Press SPACE to save the screenshots","",""}, 0, 0},
    {1, {"Saving Screenshots. Please wait.", "", "",""}, 0, 0}
};

// The uniform buffer object used in this example
struct GlobalUniformBufferObject {
    alignas(16) glm::vec3 lightDir;
    alignas(16) glm::vec4 lightColor;
    alignas(16) glm::vec3 eyePos;
    alignas(16) glm::vec4 eyeDir;
};

enum GameState {
    TEST_LIGH,
    WAITING_BOAT_X,
    WAITING_BOAT_Y,
    PROCESSING_BOAT_INPUT,
    WAITING_ATTACK_X,
    WAITING_ATTACK_Y,
    PROCESSING_ATTACK_INPUT,
    ANIMATING_MISSILE,
    ANIMATING_EXPLOSION,
};

struct UniformBufferObject {
    alignas(16) glm::mat4 mvpMat;
    alignas(16) glm::mat4 mMat;
    alignas(16) glm::mat4 nMat;
    alignas(16) glm::vec4 color;
};

struct PlaneUniformBufferObject {
    alignas(16) glm::mat4 mvpMat[NPLANE];
    alignas(16) glm::mat4 mMat[NPLANE];
    alignas(16) glm::mat4 nMat[NPLANE];
    alignas(16) glm::vec4 color[NPLANE];
};

struct skyBoxUniformBufferObject {
    alignas(16) glm::mat4 mvpMat;
};

// The vertices data structures
struct skyBoxVertex {
    glm::vec3 pos;
};

struct Vertex {
    glm::vec3 pos;
    glm::vec2 UV;
    glm::vec3 norm;
};

struct Light {
    glm::vec3 position;
    glm::vec3 color;
};

// MAIN ! 
class FinalProject : public BaseProject {
protected:

    // Descriptor Layouts ["classes" of what will be passed to the shaders]
    DescriptorSetLayout DSLGlobal;  // For Global values

    DescriptorSetLayout DSLskyBox;  // For skyBox
    DescriptorSetLayout DSLPlane;   // For the plane
    DescriptorSetLayout DSLBattleship;  // For the battleship

    // Vertex formats
    VertexDescriptor VDskyBox;
    VertexDescriptor VDPlane;
    VertexDescriptor VDBattleship;

    // Pipelines [Shader couples]
    Pipeline PskyBox;
    Pipeline PPlane;
    Pipeline PBattleship;

    // Scenes and texts
    TextMaker txt;

    // Models, textures and Descriptor Sets (values assigned to the uniforms)
    DescriptorSet DSGlobal;

    Model MskyBox;
    Texture TskyBox;
    DescriptorSet DSskyBox;

    Model MlargePlane;
    Texture TlargePlane;
    DescriptorSet DSPlane;

    Model MverticalPlaneA;  // Modello per il piano verticale
    Texture TverticalPlane;  // Texture per il piano verticale
    DescriptorSet DSVerticalPlaneA;  // Descriptor set per il piano verticale

    Model MverticalPlaneB;  // Modello per il piano verticale
    DescriptorSet DSVerticalPlaneB;  // Descriptor set per il piano verticale

    Model Mb0p0;
    Texture Tbattleship;
    DescriptorSet DSb0p0;

    Model Mb1p0;
    DescriptorSet DSb1p0;

    Model Mb0p1;
    DescriptorSet DSb0p1;

    Model Mb1p1;
    DescriptorSet DSb1p1;

    Model Mmissile;
    Texture Tmissile;
    DescriptorSet DSmissile;
    
    Model MExplosionSphere;
    Texture TExplosionSphere;
    DescriptorSet DSExplosionSphere;

    // Other application parameters
    int currScene = 0;
    int subpass = 0;
    int currPlayer = 0;

    glm::vec3 CamPos = glm::vec3(0.0f, 50.0f, 100.0f);
    glm::mat4 ViewMatrix;
    float CamAlpha = 0.0f;
    float CamBeta = 0.0f;

    float Ar;

    // Player boats data:
    // Boats coordinates
    int B0P0_x = -1;
    int B1P0_x = -1;
    int B0P1_x = -1;
    int B1P1_x = -1;
    int B0P0_y = -1;
    int B1P0_y = -1;
    int B0P1_y = -1;
    int B1P1_y = -1;
    bool B0P0Alive = true;
    bool B1P0Alive = true;
    bool B0P1Alive = true;
    bool B1P1Alive = true;

    //missile trajectory data
    glm::vec3 missileStartPos = glm::vec3(0.0f, 0.0f, 100.0f);  // Posizione iniziale sopra il piano
    glm::vec3 missileEndPos = glm::vec3(0.0f, 0.0f, 0.0f);
    float h = 20.0f;
    float missileTime = 0.0f;
    float totalTime = 3.0f;
    glm::vec3 missilePos = missileStartPos;  // Posizione corrente del missile
    GameState currentState = WAITING_BOAT_X;
    int targetX = -1;
    int targetY = -1;
    bool inputXSet = false;  // True se il targetX � stato inserito
    bool inputYSet = false;  // True se il targetY � stato inserito
    bool boatVisible = false;
    bool isMissileVisible = false;

    //Explosion variables
    glm::vec3 explosionCenter = glm::vec3(0.0f, -100.0f, 0.0f);
    float explosionRadius = 0.0f;
    float explosionMaxRadius = 1.0f;
    float explosionTime = 0.0f;
    float explosionDuration = 3.0f;
    bool isExplosionVisible = false;

    // Here you set the main application parameters
    void setWindowParameters() {
        // window size, titile and initial background
        windowWidth = 800;
        windowHeight = 600;
        windowTitle = "FinalProject - Warships";
        windowResizable = GLFW_TRUE;
        initialBackgroundColor = { 0.1f, 0.1f, 0.1f, 1.0f };

        Ar = (float)windowWidth / (float)windowHeight;
    }

    // What to do when the window changes size
    void onWindowResize(int w, int h) {
        std::cout << "Window resized to: " << w << " x " << h << "\n";
        Ar = (float)w / (float)h;
    }

    // Here you load and setup all your Vulkan Models and Texutures.
    // Here you also create your Descriptor set layouts and load the shaders for the pipelines
    void localInit() {
        // Descriptor Layouts [what will be passed to the shaders]
        DSLGlobal.init(this, {
                    {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, sizeof(GlobalUniformBufferObject), 1}
            });
        DSLskyBox.init(this, {
                    {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, sizeof(skyBoxUniformBufferObject), 1},
                    {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1},
                    {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, 1, 1}
            });
        DSLPlane.init(this, {
                    {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, sizeof(PlaneUniformBufferObject), 1},
                    {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1},
                    {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, 1, 1}
            });
        DSLBattleship.init(this, {
                    {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, sizeof(UniformBufferObject), 1},
                    {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1},
                    {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, 1, 1}
            });


        // Vertex descriptors
        VDskyBox.init(this, {
                  {0, sizeof(skyBoxVertex), VK_VERTEX_INPUT_RATE_VERTEX}
            }, {
              {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(skyBoxVertex, pos),
                     sizeof(glm::vec3), POSITION}
            });
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
        PskyBox.init(this, &VDskyBox, "shaders/SkyBoxVert.spv", "shaders/SkyBoxFrag.spv", { &DSLskyBox });
        PskyBox.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_BACK_BIT, false);

        PPlane.init(this, &VDPlane, "shaders/PlaneVert.spv", "shaders/PlaneFrag.spv", { &DSLPlane });
        PPlane.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_BACK_BIT, false);

        PBattleship.init(this, &VDBattleship, "shaders/PhongVert.spv", "shaders/PhongFrag.spv", { &DSLBattleship });
        PBattleship.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_BACK_BIT, false);

        // Create models
        MskyBox.init(this, &VDskyBox, "models/SkyBoxCube.obj", OBJ);
        MlargePlane.init(this, &VDPlane, "models/Water.obj", OBJ);
        Mb0p0.init(this, &VDBattleship, "models/Warships/Battleship.obj", OBJ);
        Mb1p0.init(this, &VDBattleship, "models/Warships/Battleship.obj", OBJ);
        Mb0p1.init(this, &VDBattleship, "models/Warships/Battleship.obj", OBJ);
        Mb1p1.init(this, &VDBattleship, "models/Warships/Battleship.obj", OBJ);
        Mmissile.init(this, &VDBattleship, "models/Missile/missile3.obj", OBJ);
        MverticalPlaneA.init(this, &VDPlane, "models/LargePlane3.obj", OBJ);  // Inizializza il modello del piano verticale
        MverticalPlaneB.init(this, &VDPlane, "models/LargePlane3.obj", OBJ);  // Inizializza il modello del piano verticale
        MExplosionSphere.init(this, &VDBattleship, "models/Sphere.obj", OBJ);

        // Create the textures
        TskyBox.init(this, "textures/starmap_g4k.jpg");
        TlargePlane.init(this, "textures/water_cropped_grid.jpg");
        Tbattleship.init(this, "textures/Metal.jpg");
        Tmissile.init(this, "textures/missile_texture.jpg");
        TverticalPlane.init(this, "textures/texvertplaneA.jpg");  // Inizializza la texture del piano verticale (sostituisci con il percorso corretto)
        TExplosionSphere.init(this, "textures/explosion_texture.jpg");

        // Descriptor pool sizes
        // WARNING!!!!!!!!
        // Must be set before initializing the text and the scene
        DPSZs.uniformBlocksInPool = 21;
        DPSZs.texturesInPool = 21;
        DPSZs.setsInPool = 21;

        std::cout << "Initializing text\n";
        txt.init(this, &outText);

        std::cout << "Initialization completed!\n";
        std::cout << "Uniform Blocks in the Pool  : " << DPSZs.uniformBlocksInPool << "\n";
        std::cout << "Textures in the Pool        : " << DPSZs.texturesInPool << "\n";
        std::cout << "Descriptor Sets in the Pool : " << DPSZs.setsInPool << "\n";

        ViewMatrix = glm::translate(glm::mat4(1), -CamPos);
    }

    // Here you create your pipelines and Descriptor Sets!
    void pipelinesAndDescriptorSetsInit() {
        // This creates a new pipeline (with the current surface), using its shaders
        PskyBox.create();
        PPlane.create();
        PBattleship.create();

        // Here you define the data set
        DSskyBox.init(this, &DSLskyBox, { &TskyBox });
        DSPlane.init(this, &DSLPlane, { &TlargePlane });
        DSb0p0.init(this, &DSLBattleship, { &Tbattleship });
        DSb1p0.init(this, &DSLBattleship, { &Tbattleship });
        DSb0p1.init(this, &DSLBattleship, { &Tbattleship });
        DSb1p1.init(this, &DSLBattleship, { &Tbattleship });
        DSmissile.init(this, &DSLBattleship, { &Tmissile });
        DSVerticalPlaneA.init(this, &DSLPlane, { &TverticalPlane });  // Inizializza il descriptor set per il piano verticale con la sua texture
        DSVerticalPlaneB.init(this, &DSLPlane, { &TverticalPlane });
        DSExplosionSphere.init(this, &DSLBattleship, { &TExplosionSphere });

        DSGlobal.init(this, &DSLGlobal, {});

        txt.pipelinesAndDescriptorSetsInit();
    }

    // Here you destroy your pipelines and Descriptor Sets!
    // All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
    void pipelinesAndDescriptorSetsCleanup() {
        // Cleanup pipelines
        PskyBox.cleanup();
        PPlane.cleanup();
        PBattleship.cleanup();

        DSskyBox.cleanup();
        DSPlane.cleanup();
        DSb0p0.cleanup();
        DSb1p0.cleanup();
        DSb0p1.cleanup();
        DSb1p1.cleanup();
        DSmissile.cleanup();
        DSGlobal.cleanup();
        DSVerticalPlaneA.cleanup();
        DSVerticalPlaneB.cleanup();
        DSExplosionSphere.cleanup();

        txt.pipelinesAndDescriptorSetsCleanup();
    }

    // Here you destroy all the Models, Texture and Desc. Set Layouts you created!
    // All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
    // You also have to destroy the pipelines: since they need to be rebuilt, they have two different
    // methods: .cleanup() recreates them, while .destroy() delete them completely
    void localCleanup() {
        TskyBox.cleanup();
        MskyBox.cleanup();

        TlargePlane.cleanup();
        MlargePlane.cleanup();

        Tbattleship.cleanup();
        Mb0p0.cleanup();
        Mb1p0.cleanup();
        Mb0p1.cleanup();
        Mb1p1.cleanup();

        TverticalPlane.cleanup();
        MverticalPlaneA.cleanup();
        MverticalPlaneB.cleanup();

        Tmissile.cleanup();
        Mmissile.cleanup();

        MExplosionSphere.cleanup();
        TExplosionSphere.cleanup();

        // Cleanup descriptor set layouts
        DSLGlobal.cleanup();
        DSLskyBox.cleanup();
        DSLPlane.cleanup();
        DSLBattleship.cleanup();
        //DSmissile.cleanup();

        // Destroies the pipelines
        PskyBox.destroy();
        PPlane.destroy();
        PBattleship.destroy();

        txt.localCleanup();
    }

    // Here it is the creation of the command buffer:
    // You send to the GPU all the objects you want to draw,
    // with their buffers and textures
    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
        PskyBox.bind(commandBuffer);
        MskyBox.bind(commandBuffer);
        DSskyBox.bind(commandBuffer, PskyBox, 0, currentImage);
        vkCmdDrawIndexed(commandBuffer,
            static_cast<uint32_t>(MskyBox.indices.size()), 1, 0, 0, 0);

        // Bind the pipeline for the plane
        PPlane.bind(commandBuffer);
        MlargePlane.bind(commandBuffer);
        DSPlane.bind(commandBuffer, PPlane, 0, currentImage);
        // Draw the plane
        vkCmdDrawIndexed(commandBuffer,
            static_cast<uint32_t>(MlargePlane.indices.size()), NPLANE, 0, 0, 0);

        // Bind the pipeline for the battleship 
        PBattleship.bind(commandBuffer);
        Mb0p0.bind(commandBuffer);
        DSb0p0.bind(commandBuffer, PBattleship, 0, currentImage);
        // Draw the battleship
        vkCmdDrawIndexed(commandBuffer,
            static_cast<uint32_t>(Mb0p0.indices.size()), 1, 0, 0, 0);

        // Bind the pipeline for the Smallbattleship
        Mb1p0.bind(commandBuffer);
        DSb1p0.bind(commandBuffer, PBattleship, 0, currentImage);
        // Draw the battleship
        vkCmdDrawIndexed(commandBuffer,
            static_cast<uint32_t>(Mb1p0.indices.size()), 1, 0, 0, 0);

        // Bind the pipeline for the Smallbattleship
        Mb0p1.bind(commandBuffer);
        DSb0p1.bind(commandBuffer, PBattleship, 0, currentImage);
        // Draw the battleship
        vkCmdDrawIndexed(commandBuffer,
            static_cast<uint32_t>(Mb0p1.indices.size()), 1, 0, 0, 0);

        // Bind the pipeline for the Smallbattleship
        Mb1p1.bind(commandBuffer);
        DSb1p1.bind(commandBuffer, PBattleship, 0, currentImage);
        // Draw the battleship
        vkCmdDrawIndexed(commandBuffer,
            static_cast<uint32_t>(Mb1p1.indices.size()), 1, 0, 0, 0);

        //missile
        Mmissile.bind(commandBuffer);
        DSmissile.bind(commandBuffer, PBattleship, 0, currentImage);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(Mmissile.indices.size()), 1, 0, 0, 0);

        // Vertical plane
        MverticalPlaneA.bind(commandBuffer);
        DSVerticalPlaneA.bind(commandBuffer, PPlane, 0, currentImage);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(MverticalPlaneA.indices.size()), 1, 0, 0, 0);
        MverticalPlaneB.bind(commandBuffer);
        DSVerticalPlaneB.bind(commandBuffer, PPlane, 0, currentImage);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(MverticalPlaneB.indices.size()), 1, 0, 0, 0);
        
        MExplosionSphere.bind(commandBuffer);
        DSExplosionSphere.bind(commandBuffer, PBattleship, 0, currentImage);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(MExplosionSphere.indices.size()), 1, 0, 0, 0);

        txt.populateCommandBuffer(commandBuffer, currentImage, currScene);
    }

    // Here is where you update the uniforms.
    // Very likely this will be where you will be writing the logic of your application.
    void updateUniformBuffer(uint32_t currentImage) {
        static bool debounce = false;
        static int curDebounce = 0;

        int S = 9;  // Numero di righe
        int T = 9;  // Numero di colonne
        // Creiamo una matrice S x T di glm::mat4
        std::vector<std::vector<glm::mat4>> matrix(S, std::vector<glm::mat4>(T));
        std::vector<std::vector<glm::mat4>> matrixB(S, std::vector<glm::mat4>(T));

        // Inizializzazione degli elementi della matrice
        for (int i = 0; i < S; ++i) {
            for (int j = 0; j < T; ++j) {
                matrix[i][j] = glm::mat4(1.0f); // Inizializza ogni glm::mat4 come matrice identità
                matrixB[i][j] = glm::mat4(1.0f);
            }
        }

        // Ora costruiamo la scacchiera con le rispettive matrici
        for (int i = 0; i < S; ++i) {
            for (int j = 0; j < T; ++j) {
                matrix[i][j] = glm::translate(glm::mat4(1.0f), glm::vec3(22.0f * (j - 4), 0.0f, 22.0f * (i - 4)));
                matrixB[i][j] = glm::translate(glm::mat4(1.0f), glm::vec3(22.0f * (j - 4), 0.0f, 22.0f * (i - 4)));
                matrixB[i][j] = glm::rotate(matrixB[i][j], glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                matrixB[i][j] = glm::translate(matrixB[i][j], glm::vec3(0.0f, 0.0f, 193.0f));

            }
        }

        //glm::mat4(1.0f) traslata di 1.5 rispetto all'asse x è il centro della scacchiera
        //Ogni elemento della colonna 5 ha una traslazione x di 1.5f
        //Ogni elemento si muove a multipli di 21.3f rispetto alla z

        float deltaT;
        glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
        bool fire = false;
        getSixAxis(deltaT, m, r, fire);

        const float ROT_SPEED = glm::radians(120.0f);
        const float MOVE_SPEED = 30.0f; //If you want to move faster, increase this value

        // The Fly model update proc.
        CamAlpha = CamAlpha - ROT_SPEED * deltaT * r.y;
        CamBeta = CamBeta - ROT_SPEED * deltaT * r.x;
        CamBeta = CamBeta < glm::radians(-90.0f) ? glm::radians(-90.0f) :
            (CamBeta > glm::radians(90.0f) ? glm::radians(90.0f) : CamBeta);

        glm::vec3 ux = glm::rotate(glm::mat4(1.0f), CamAlpha, glm::vec3(0, 1, 0)) * glm::vec4(1, 0, 0, 1);
        glm::vec3 uz = glm::rotate(glm::mat4(1.0f), CamAlpha, glm::vec3(0, 1, 0)) * glm::vec4(0, 0, 1, 1);
        CamPos = CamPos + MOVE_SPEED * m.x * ux * deltaT;
        CamPos = CamPos + MOVE_SPEED * m.y * glm::vec3(0, 1, 0) * deltaT;
        CamPos = CamPos + MOVE_SPEED * m.z * uz * deltaT;

        static float subpassTimer = 0.0;

        if (glfwGetKey(window, GLFW_KEY_SPACE)) {
            if (!debounce) {
                debounce = true;
                curDebounce = GLFW_KEY_SPACE;
                if (currScene != 1) {
                    currScene = (currScene + 1) % outText.size();

                }
                if (currScene == 1) {
                    if (subpass >= 4) {
                        currScene = 0;
                    }
                }
                std::cout << "Scene : " << currScene << "\n";

                RebuildPipeline();
            }
        }
        else {
            if ((curDebounce == GLFW_KEY_SPACE) && debounce) {
                debounce = false;
                curDebounce = 0;
            }
        }

        // Standard procedure to quit when the ESC key is pressed
        if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }

        /*Lo lascio potrebbe servire dopo per le macchine di gioco
        if (glfwGetKey(window, GLFW_KEY_V)) {
            if (!debounce) {
                debounce = true;
                curDebounce = GLFW_KEY_V;

                printMat4("ViewMatrix  ", ViewMatrix);
            }
        }
        else {
            if ((curDebounce == GLFW_KEY_V) && debounce) {
                debounce = false;
                curDebounce = 0;
            }
        }

        if (glfwGetKey(window, GLFW_KEY_C)) {
            if (!debounce) {
                debounce = true;
                curDebounce = GLFW_KEY_C;
            }
        }
        else {
            if ((curDebounce == GLFW_KEY_C) && debounce) {
                debounce = false;
                curDebounce = 0;
            }
        }

        if (glfwGetKey(window, GLFW_KEY_T)) {
            if (!debounce) {
                debounce = true;
                curDebounce = GLFW_KEY_T;
            }
        }
        else {
            if ((curDebounce == GLFW_KEY_T) && debounce) {
                debounce = false;
                curDebounce = 0;
            }
        }*/


        if (currScene == 1) {
            switch (subpass) {
            case 0:
                ViewMatrix = glm::mat4(-0.0656882, -0.162777, 0.984474, 0, 0.0535786, 0.984606, 0.166374, 0, -0.996401, 0.0636756, -0.0559558, 0, 0.0649244, -0.531504, -3.26128, 1);
                break;
            case 1:
                ViewMatrix = glm::mat4(-0.312507, -0.442291, 0.840666, 0, 0.107287, 0.862893, 0.493868, 0, -0.943837, 0.24453, -0.222207, 0, -0.0157694, -0.186147, -1.54649, 1);
                break;
            case 2:
                ViewMatrix = glm::mat4(-0.992288, 0.00260993, -0.12393, 0, -0.0396232, 0.940648, 0.337063, 0, 0.117454, 0.339374, -0.93329, 0, 0.0335061, -0.0115242, -2.99662, 1);
                break;
            case 3:
                ViewMatrix = glm::mat4(0.0942192, -0.242781, 0.965495, 0, 0.560756, 0.814274, 0.150033, 0, -0.822603, 0.527272, 0.212861, 0, -0.567191, -0.254532, -1.79143, 1);
                break;
            }
        }

        if (currScene == 1) {
            subpassTimer += deltaT;
            if (subpassTimer > 4.0f) {
                subpassTimer = 0.0f;
                subpass++;
                std::cout << "Scene : " << currScene << " subpass: " << subpass << "\n";
                char buf[20];
                sprintf(buf, "FinalProject_%d.png", subpass);
                saveScreenshot(buf, currentImage);
                if (subpass == 4) {
                    ViewMatrix = glm::translate(glm::mat4(1), -CamPos);


                    currScene = 0;
                    std::cout << "Scene : " << currScene << "\n";
                    RebuildPipeline();
                }
            }
        }

        // Here is where you actually update your uniforms
        glm::mat4 M = glm::perspective(glm::radians(45.0f), Ar, 0.1f, 1000.0f); // Projection matrix; If you want to see further icrease the last parameter
        M[1][1] *= -1;

        glm::mat4 Mv = glm::rotate(glm::mat4(1.0), -CamBeta, glm::vec3(1, 0, 0)) *
            glm::rotate(glm::mat4(1.0), -CamAlpha, glm::vec3(0, 1, 0)) *
            glm::translate(glm::mat4(1.0), -CamPos);

        glm::mat4 ViewPrj = M * Mv;
        glm::mat4 baseTr = glm::mat4(1.0f);

        // updates global uniforms
        // Global
        GlobalUniformBufferObject gubo{};
        gubo.lightDir = glm::vec3(cos(glm::radians(135.0f)), sin(glm::radians(135.0f)), 0.0f);
        gubo.lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        gubo.eyePos = glm::vec3(glm::inverse(ViewMatrix) * glm::vec4(0, 0, 0, 1));
        gubo.eyeDir = glm::vec4(0, 0, 0, 1) * ViewMatrix;
        DSGlobal.map(currentImage, &gubo, 0);

        PlaneUniformBufferObject pubo{};

        // Update uniforms for the plane
        pubo.mMat[0] = glm::mat4(1.0f);
        pubo.mvpMat[0] = ViewPrj * pubo.mMat[0];
        pubo.nMat[0] = glm::inverse(glm::transpose(pubo.mMat[0]));
        pubo.color[0] = glm::vec4(1.0f);

        pubo.mMat[1] = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -195.0f));
        pubo.mvpMat[1] = ViewPrj * pubo.mMat[1];
        pubo.nMat[1] = glm::inverse(glm::transpose(pubo.mMat[1]));
        pubo.color[1] = glm::vec4(1.0f);

        DSPlane.map(currentImage, &pubo, 0);
        DSPlane.map(currentImage, &gubo, 2);

        skyBoxUniformBufferObject sbubo{};
        sbubo.mvpMat = M * glm::mat4(glm::mat3(Mv));
        DSskyBox.map(currentImage, &sbubo, 0);

        //For the vertical plane
        UniformBufferObject uboVerticalPlaneA{};
        UniformBufferObject uboVerticalPlaneB{};
        uboVerticalPlaneA.mMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        uboVerticalPlaneA.mvpMat = ViewPrj * uboVerticalPlaneA.mMat;
        uboVerticalPlaneA.nMat = glm::inverse(glm::transpose(uboVerticalPlaneA.mMat));
        uboVerticalPlaneA.color = glm::vec4(1.0f);  // Colore del piano verticale

        uboVerticalPlaneB.mMat = glm::translate(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec3(-194.0f, 0.0f, 0.0f));
        uboVerticalPlaneB.mvpMat = ViewPrj * uboVerticalPlaneB.mMat;
        uboVerticalPlaneB.nMat = glm::inverse(glm::transpose(uboVerticalPlaneB.mMat));
        uboVerticalPlaneB.color = glm::vec4(1.0f);  // Colore del piano verticale

        DSVerticalPlaneA.map(currentImage, &uboVerticalPlaneA, 0);
        DSVerticalPlaneA.map(currentImage, &gubo, 2);
        DSVerticalPlaneB.map(currentImage, &uboVerticalPlaneB, 0);
        DSVerticalPlaneB.map(currentImage, &gubo, 2);

        UniformBufferObject uboMissile{};
        UniformBufferObject uboExplosion{};
        //FSA che gestisce le fasi di gioco
        switch (currentState) {
            case WAITING_BOAT_X: {
                if (currPlayer == 0) {
                    if (B0P0_x == -1) {
                        if (!debounce) {
                            if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) { B0P0_x = 0; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) { B0P0_x = 1; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) { B0P0_x = 2; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) { B0P0_x = 3; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) { B0P0_x = 4; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) { B0P0_x = 5; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) { B0P0_x = 6; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) { B0P0_x = 7; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) { B0P0_x = 8; debounce = true; }

                            if (B0P0_x != -1) {
                                std::cout << "B0P0_x set to: " << B0P0_x << "\n";
                                currentState = WAITING_BOAT_Y;
                            }
                        }
                        if (glfwGetKey(window, GLFW_KEY_0) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_5) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_6) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_7) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_8) == GLFW_RELEASE) {
                            debounce = false;  // Reset debounce
                        }
                    }
                    else {
                        if (!debounce) {
                            if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) { B1P0_x = 0; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) { B1P0_x = 1; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) { B1P0_x = 2; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) { B1P0_x = 3; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) { B1P0_x = 4; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) { B1P0_x = 5; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) { B1P0_x = 6; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) { B1P0_x = 7; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) { B1P0_x = 8; debounce = true; }

                            if (B1P0_x != -1) {
                                std::cout << "B1P0_x set to: " << B1P0_x << "\n";
                                currentState = WAITING_BOAT_Y;
                            }
                        }
                        if (glfwGetKey(window, GLFW_KEY_0) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_5) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_6) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_7) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_8) == GLFW_RELEASE) {
                            debounce = false;  // Reset debounce
                        }
                    }
                }
                else {
                    if (B0P1_x == -1) {
                        if (!debounce) {
                            if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) { B0P1_x = 0; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) { B0P1_x = 1; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) { B0P1_x = 2; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) { B0P1_x = 3; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) { B0P1_x = 4; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) { B0P1_x = 5; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) { B0P1_x = 6; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) { B0P1_x = 7; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) { B0P1_x = 8; debounce = true; }

                            if (B0P1_x != -1) {
                                std::cout << "B0P1_x set to: " << B0P1_x << "\n";
                                currentState = WAITING_BOAT_Y;
                            }
                        }
                        if (glfwGetKey(window, GLFW_KEY_0) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_5) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_6) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_7) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_8) == GLFW_RELEASE) {
                            debounce = false;  // Reset debounce
                        }
                    }
                    else {
                        if (!debounce) {
                            if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) { B1P1_x = 0; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) { B1P1_x = 1; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) { B1P1_x = 2; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) { B1P1_x = 3; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) { B1P1_x = 4; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) { B1P1_x = 5; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) { B1P1_x = 6; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) { B1P1_x = 7; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) { B1P1_x = 8; debounce = true; }

                            if (B1P1_x != -1) {
                                std::cout << "B1P1_x set to: " << B1P1_x << "\n";
                                currentState = WAITING_BOAT_Y;
                            }
                        }
                        if (glfwGetKey(window, GLFW_KEY_0) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_5) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_6) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_7) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_8) == GLFW_RELEASE) {
                            debounce = false;  // Reset debounce
                        }
                    }
                }
                break;
            }

            case WAITING_BOAT_Y: {
                if (currPlayer == 0) {
                    if (B0P0_y == -1) {
                        if (!debounce) {
                            if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) { B0P0_y = 0; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) { B0P0_y = 1; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) { B0P0_y = 2; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) { B0P0_y = 3; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) { B0P0_y = 4; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) { B0P0_y = 5; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) { B0P0_y = 6; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) { B0P0_y = 7; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) { B0P0_y = 8; debounce = true; }

                            if (B0P0_y != -1) {
                                std::cout << "B0P0_y set to: " << B0P0_y << "\n";
                                currentState = WAITING_BOAT_X;
                            }
                        }
                        if (glfwGetKey(window, GLFW_KEY_0) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_5) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_6) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_7) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_8) == GLFW_RELEASE) {
                            debounce = false;  // Reset debounce
                        }
                    }
                    else {
                        if (!debounce) {
                            if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) { B1P0_y = 0; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) { B1P0_y = 1; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) { B1P0_y = 2; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) { B1P0_y = 3; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) { B1P0_y = 4; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) { B1P0_y = 5; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) { B1P0_y = 6; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) { B1P0_y = 7; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) { B1P0_y = 8; debounce = true; }

                            if (B1P0_y != -1) {
                                std::cout << "B1P0_y set to: " << B1P0_y << "\n";
                                currentState = WAITING_BOAT_X;
                                currPlayer = 1;
                            }
                        }
                        if (glfwGetKey(window, GLFW_KEY_0) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_5) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_6) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_7) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_8) == GLFW_RELEASE) {
                            debounce = false;  // Reset debounce
                        }
                    }
                }
                else {
                    if (B0P1_y == -1) {
                        if (!debounce) {
                            if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) { B0P1_y = 0; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) { B0P1_y = 1; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) { B0P1_y = 2; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) { B0P1_y = 3; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) { B0P1_y = 4; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) { B0P1_y = 5; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) { B0P1_y = 6; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) { B0P1_y = 7; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) { B0P1_y = 8; debounce = true; }

                            if (B0P1_y != -1) {
                                std::cout << "B0P1_y set to: " << B0P1_y << "\n";
                                currentState = WAITING_BOAT_X;
                            }
                        }
                        if (glfwGetKey(window, GLFW_KEY_0) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_5) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_6) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_7) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_8) == GLFW_RELEASE) {
                            debounce = false;  // Reset debounce
                        }
                    }
                    else {
                        if (!debounce) {
                            if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) { B1P1_y = 0; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) { B1P1_y = 1; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) { B1P1_y = 2; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) { B1P1_y = 3; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) { B1P1_y = 4; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) { B1P1_y = 5; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) { B1P1_y = 6; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) { B1P1_y = 7; debounce = true; }
                            if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) { B1P1_y = 8; debounce = true; }

                            if (B1P1_y != -1) {
                                std::cout << "B1P1_y set to: " << B1P1_y << "\n";
                                currentState = PROCESSING_BOAT_INPUT;
                                currPlayer = 0;
                            }
                        }
                        if (glfwGetKey(window, GLFW_KEY_0) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_5) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_6) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_7) == GLFW_RELEASE &&
                            glfwGetKey(window, GLFW_KEY_8) == GLFW_RELEASE) {
                            debounce = false;  // Reset debounce
                        }
                    }
                }
                break;
            }

            case PROCESSING_BOAT_INPUT: {
                UniformBufferObject ubo{};
                // Update uniforms for the battleship
                ubo.mMat = matrix[B0P0_x][B0P0_y];
                ubo.mvpMat = ViewPrj * ubo.mMat;
                ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
                ubo.color = glm::vec4(1.0f);

                DSb0p0.map(currentImage, &ubo, 0);
                DSb0p0.map(currentImage, &gubo, 2);


                // Update uniforms for the battleship
                ubo.mMat = matrix[B1P0_x][B1P0_y];
                ubo.mvpMat = ViewPrj * ubo.mMat;
                ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
                ubo.color = glm::vec4(1.0f);

                DSb1p0.map(currentImage, &ubo, 0);
                DSb1p0.map(currentImage, &gubo, 2);

                // TODO: mancano le battelship del giocatore 1 e vanno aggiunte mappandole sulla seconda tavola
                ubo.mMat = matrixB[B0P1_x][B0P1_y];
                ubo.mvpMat = ViewPrj * ubo.mMat;
                ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
                ubo.color = glm::vec4(1.0f);

                DSb0p1.map(currentImage, &ubo, 0);
                DSb0p1.map(currentImage, &gubo, 2);

                ubo.mMat = matrixB[B1P1_x][B1P1_y];
                ubo.mvpMat = ViewPrj * ubo.mMat;
                ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
                ubo.color = glm::vec4(1.0f);

                DSb1p1.map(currentImage, &ubo, 0);
                DSb1p1.map(currentImage, &gubo, 2);

                currentState = WAITING_ATTACK_X;
                boatVisible = true;
                break;
            }

            case WAITING_ATTACK_X: {
                if (!debounce) {
                    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) { targetX = 0; debounce = true; }
                    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) { targetX = 1; debounce = true; }
                    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) { targetX = 2; debounce = true; }
                    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) { targetX = 3; debounce = true; }
                    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) { targetX = 4; debounce = true; }
                    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) { targetX = 5; debounce = true; }
                    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) { targetX = 6; debounce = true; }
                    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) { targetX = 7; debounce = true; }
                    if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) { targetX = 8; debounce = true; }

                    if (targetX != -1) {
                        inputXSet = true;  // La coordinata X è stata inserita
                        std::cout << "X set to: " << targetX << "\n";
                        currentState = WAITING_ATTACK_Y;
                    }
                }
                if (glfwGetKey(window, GLFW_KEY_0) == GLFW_RELEASE &&
                    glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE &&
                    glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE &&
                    glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE &&
                    glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE &&
                    glfwGetKey(window, GLFW_KEY_5) == GLFW_RELEASE &&
                    glfwGetKey(window, GLFW_KEY_6) == GLFW_RELEASE &&
                    glfwGetKey(window, GLFW_KEY_7) == GLFW_RELEASE &&
                    glfwGetKey(window, GLFW_KEY_8) == GLFW_RELEASE) {
                    debounce = false;  // Reset debounce
                }
                break;

            }

            case WAITING_ATTACK_Y: {
                if (!debounce) {
                    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) { targetY = 0; debounce = true; }
                    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) { targetY = 1; debounce = true; }
                    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) { targetY = 2; debounce = true; }
                    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) { targetY = 3; debounce = true; }
                    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) { targetY = 4; debounce = true; }
                    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) { targetY = 5; debounce = true; }
                    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) { targetY = 6; debounce = true; }
                    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) { targetY = 7; debounce = true; }
                    if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) { targetY = 8; debounce = true; }

                    if (targetY != -1) {
                        inputYSet = true;  // La coordinata Y è stata inserita
                        std::cout << "Y set to: " << targetY << "\n";
                        currentState = PROCESSING_ATTACK_INPUT;
                    }
                }
                if (glfwGetKey(window, GLFW_KEY_0) == GLFW_RELEASE &&
                    glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE &&
                    glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE &&
                    glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE &&
                    glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE &&
                    glfwGetKey(window, GLFW_KEY_5) == GLFW_RELEASE &&
                    glfwGetKey(window, GLFW_KEY_6) == GLFW_RELEASE &&
                    glfwGetKey(window, GLFW_KEY_7) == GLFW_RELEASE &&
                    glfwGetKey(window, GLFW_KEY_8) == GLFW_RELEASE) {
                    debounce = false;  // Reset debounce
                }
                break;
            }

            case PROCESSING_ATTACK_INPUT: {
                if (targetX != -1 && targetY != -1) {
                    missileTime = 0.0f;  // Reset the missile animation time
                    if (currPlayer == 0) {
                        // TODO - settare l'arrivo in base alla matrice che mappa il secondo tabellone  
                        glm::mat4 targetMatrix = matrixB[targetX][targetY];
                        glm::vec3 targetPosition = glm::vec3(targetMatrix[3]);
                        missileEndPos = targetPosition;
                    }
                    else {

                        glm::mat4 targetMatrix = matrix[targetX][targetY];
                        glm::vec3 targetPosition = glm::vec3(targetMatrix[3]);
                        missileEndPos = targetPosition;
                    }
                    // Reset degli input per il prossimo round
                    h = length(missileEndPos - missileStartPos) * 0.4;
                    currentState = ANIMATING_MISSILE;
                }
                break;
            }

            case ANIMATING_MISSILE: {
                isMissileVisible = true;

                //Start position function
                missileStartPos = glm::vec3(0.0f, 0.0f, 100.0f + (-400.0f * currPlayer));

                // Aggiorna la posizione del missile
                missileTime += deltaT / totalTime; // Aggiorna il tempo normalizzato (0 -> 1)

                // Clamp del tempo per assicurarsi che rimanga nell'intervallo [0, 1]
                missileTime = glm::clamp(missileTime, 0.0f, 1.0f);

                // Calcola la nuova posizione del missile
                float t = missileTime;
                float x = missileStartPos.x + t * (missileEndPos.x - missileStartPos.x);
                float y = missileStartPos.y + 4 * h * t * (1.0f - t);  // Vertical component for the parabolic arc
                float z = missileStartPos.z + t * (missileEndPos.z - missileStartPos.z);  // Horizontal direction

                missilePos = glm::vec3(x, y, z);

                glm::vec3 velocity = glm::normalize(glm::vec3(
                    missileEndPos.x - missileStartPos.x,
                    4 * h * (1.0f - 2.0f * t),  // Derivative of the vertical parabolic function
                    missileEndPos.z - missileStartPos.z
                ));

                // Calculate the right vector using a cross product with an up vector
                glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);  // Up vector
                glm::vec3 right = glm::normalize(glm::cross(up, velocity));

                // Recalculate the up vector to ensure orthogonality
                up = glm::normalize(glm::cross(velocity, right));

                // Create the rotation matrix to orient the missile
                glm::mat4 rotationMatrix = glm::mat4(glm::vec4(right, 0.0f),
                    glm::vec4(up, 0.0f),
                    glm::vec4(velocity, 0.0f),
                    glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

                // Create the transformation matrix for the missile
                glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), missilePos);
                glm::mat4 modelMatrix = translationMatrix * rotationMatrix;

                // Se il missile ha raggiunto il punto finale, ferma l'animazione
                if (missileTime >= 1.0f || glm::length(missilePos - missileEndPos) < 0.1) {
                    missilePos = glm::vec3(0.0f, -100.0f, 0.0f);
                    translationMatrix = glm::translate(glm::mat4(1.0f), missilePos);
                    modelMatrix = translationMatrix * rotationMatrix;
                    uboMissile.mMat = modelMatrix;  // Apply the transformation
                    uboMissile.mvpMat = ViewPrj * uboMissile.mMat;
                    uboMissile.nMat = glm::inverse(glm::transpose(uboMissile.mMat));
                    uboMissile.color = glm::vec4(1.0f);

                    isMissileVisible = false;
                    currPlayer = (currPlayer + 1) % 2;
                    std::cout << "currentPlayer = " << currPlayer << '\n';
                    currentState = WAITING_ATTACK_X;  // Torna allo stato di attesa per nuove coordinate
                    if (currPlayer == 1) {
                        CamPos = glm::vec3(0.0f, 50.0f, -300.0f);  // Posizione dietro al vertical plane
                        ViewMatrix = glm::lookAt(CamPos, glm::vec3(0.0f, 50.0f, 100.0f), glm::vec3(0.0f, 1.0f, 0.0f));  // TODO: non guarda verso il piano verticale
                        if (targetX == B0P1_x && targetY == B0P1_y) {
                            B0P1Alive = false;
                            isExplosionVisible = true;
                            explosionCenter = missileEndPos;
                            explosionRadius = 0.0f;
                            explosionTime = 0.0f;
                            currentState = ANIMATING_EXPLOSION;
                        }
                        else if (targetX == B1P1_x && targetY == B1P1_y) {
                            B1P1Alive = false;
                            isExplosionVisible = true;
                            explosionCenter = missileEndPos;
                            explosionRadius = 0.0f;
                            explosionTime = 0.0f;
                            currentState = ANIMATING_EXPLOSION;
                        }
                    }
                    else {
                        CamPos = glm::vec3(0.0f, 50.0f, 100.0f);
                        ViewMatrix = glm::lookAt(CamPos, glm::vec3(0.0f, 50.0f, -300.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // TODO: non guarda verso il piano verticale
                        if (targetX == B0P0_x && targetY == B0P0_y) {
                            B0P0Alive = false;
                            isExplosionVisible = true;
                            explosionCenter = missileEndPos;
                            explosionRadius = 0.0f;
                            explosionTime = 0.0f;
                            currentState = ANIMATING_EXPLOSION;
                        }
                        else if (targetX == B1P0_x && targetY == B1P0_y) {
                            B1P0Alive = false;
                            isExplosionVisible = true;
                            explosionCenter = missileEndPos;
                            explosionRadius = 0.0f;
                            explosionTime = 0.0f;
                            currentState = ANIMATING_EXPLOSION;
                        }
                    }
                    inputXSet = false;
                    inputYSet = false;
                    targetX = -1;
                    targetY = -1;
                }
                else {
                    CamPos = missilePos + glm::vec3(0.0f, 4.0f, 0.0f);
                    glm::vec3 cameraTarget = missilePos + velocity;
                    ViewMatrix = glm::lookAt(CamPos, cameraTarget, up);
                    // Update the uniform buffer for the missile
                    uboMissile.mMat = modelMatrix;  // Apply the transformation
                    uboMissile.mvpMat = ViewPrj * uboMissile.mMat;
                    uboMissile.nMat = glm::inverse(glm::transpose(uboMissile.mMat));
                    uboMissile.color = glm::vec4(1.0f);

                    DSmissile.map(currentImage, &uboMissile, 0);
                    DSmissile.map(currentImage, &gubo, 2);
                }
                break;
            }

            case ANIMATING_EXPLOSION: {
                explosionTime += deltaT;
                explosionRadius = glm::mix(0.0f, explosionMaxRadius, explosionTime / explosionDuration);

                // Modifica della posizione e dell'orientamento della videocamera
                CamPos = explosionCenter + glm::vec3(0.0f, 50.0f, 50.0f);
                ViewMatrix = glm::lookAt(CamPos, explosionCenter, glm::vec3(0.0f, 1.0f, 0.0f));

                if (explosionTime >= explosionDuration) {
                    explosionCenter = glm::vec3(0.0f, -100.0f, 0.0f);
                    explosionRadius = 0.0f;
                    currentState = WAITING_ATTACK_X;
                    isExplosionVisible = false;

                    if (currPlayer == 1) {
                        CamPos = glm::vec3(0.0f, 50.0f, -300.0f);  // Posizione dietro al vertical plane
                        ViewMatrix = glm::lookAt(CamPos, glm::vec3(0.0f, 50.0f, 100.0f), glm::vec3(0.0f, 1.0f, 0.0f));  // TODO: non guarda verso il piano verticale
                    }
                    else {
                        CamPos = glm::vec3(0.0f, 50.0f, 100.0f);
                        ViewMatrix = glm::lookAt(CamPos, glm::vec3(0.0f, 50.0f, -300.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // TODO: non guarda verso il piano verticale
                    }
                }

                // Aggiorna la matrice di trasformazione della sfera dell'esplosione
                glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), explosionCenter) *
                    glm::scale(glm::mat4(1.0f), glm::vec3(explosionRadius));

                uboExplosion.mMat = modelMatrix;
                uboExplosion.mvpMat = ViewPrj * uboExplosion.mMat;
                uboExplosion.nMat = glm::inverse(glm::transpose(uboExplosion.mMat));
                uboExplosion.color = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f); // Colore arancione

                DSExplosionSphere.map(currentImage, &uboExplosion, 0);
                DSExplosionSphere.map(currentImage, &gubo, 2);                
            }
        }

        // renderizza fuori campo il missile nel caso non sia visibile
        if (!isMissileVisible) {
            DSmissile.map(currentImage, &uboMissile, 0);
            DSmissile.map(currentImage, &gubo, 2);
        }

        if (!isExplosionVisible) {
            DSExplosionSphere.map(currentImage, &uboExplosion, 0);
            DSExplosionSphere.map(currentImage, &gubo, 2);
        }

        UniformBufferObject ubo{};
        if (boatVisible) {
            // Update uniforms for the battleship
            if (B0P0Alive || currentState == ANIMATING_EXPLOSION) {
                ubo.mMat = matrix[B0P0_x][B0P0_y];
            }
            else {
                ubo.mMat = glm::translate(matrix[B0P0_x][B0P0_y], glm::vec3(0.0f, -100.0f, 0.0f));
            }

            ubo.mvpMat = ViewPrj * ubo.mMat;
            ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
            ubo.color = glm::vec4(1.0f);

            DSb0p0.map(currentImage, &ubo, 0);
            DSb0p0.map(currentImage, &gubo, 2);

            // Update uniforms for the battleship
            if (B1P0Alive || currentState == ANIMATING_EXPLOSION) {
                ubo.mMat = matrix[B1P0_x][B1P0_y];
            }
            else {
                ubo.mMat = glm::translate(matrix[B1P0_x][B1P0_y], glm::vec3(0.0f, -100.0f, 0.0f));
            }

            ubo.mvpMat = ViewPrj * ubo.mMat;
            ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
            ubo.color = glm::vec4(1.0f);

            DSb1p0.map(currentImage, &ubo, 0);
            DSb1p0.map(currentImage, &gubo, 2);

            // TODO: mancano le battelship del giocatore 1 e vanno aggiunte mappandole sulla seconda tavola
            if (B0P1Alive || currentState == ANIMATING_EXPLOSION) {
                ubo.mMat = matrixB[B0P1_x][B0P1_y];
            }
            else {
                ubo.mMat = glm::translate(matrixB[B0P1_x][B0P1_y], glm::vec3(0.0f, -100.0f, 0.0f));
            }

            ubo.mvpMat = ViewPrj * ubo.mMat;
            ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
            ubo.color = glm::vec4(1.0f);

            DSb0p1.map(currentImage, &ubo, 0);
            DSb0p1.map(currentImage, &gubo, 2);

            if (B1P1Alive || currentState == ANIMATING_EXPLOSION) {
                ubo.mMat = matrixB[B1P1_x][B1P1_y];
            }
            else {
                ubo.mMat = glm::translate(matrixB[B1P1_x][B1P1_y], glm::vec3(0.0f, -100.0f, 0.0f));
            }
            ubo.mvpMat = ViewPrj * ubo.mMat;
            ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
            ubo.color = glm::vec4(1.0f);

            DSb1p1.map(currentImage, &ubo, 0);
            DSb1p1.map(currentImage, &gubo, 2);
        }
        else { // posizioni iniziali causali - TODO: inserire pure quelle del giocatore 2
            ubo.mMat = matrix[1][3];
            ubo.mvpMat = ViewPrj * ubo.mMat;
            ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
            ubo.color = glm::vec4(1.0f);

            DSb0p0.map(currentImage, &ubo, 0);
            DSb0p0.map(currentImage, &gubo, 2);

            // Update uniforms for the battleship
            ubo.mMat = matrix[8][6];
            ubo.mvpMat = ViewPrj * ubo.mMat;
            ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
            ubo.color = glm::vec4(1.0f);

            DSb1p0.map(currentImage, &ubo, 0);
            DSb1p0.map(currentImage, &gubo, 2);

            // TODO: mancano le battelship del giocatore 1 e vanno aggiunte mappandole sulla seconda tavola
            ubo.mMat = matrixB[0][2];
            ubo.mvpMat = ViewPrj * ubo.mMat;
            ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
            ubo.color = glm::vec4(1.0f);

            DSb0p1.map(currentImage, &ubo, 0);
            DSb0p1.map(currentImage, &gubo, 2);

            ubo.mMat = matrixB[7][5];
            ubo.mvpMat = ViewPrj * ubo.mMat;
            ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
            ubo.color = glm::vec4(1.0f);

            DSb1p1.map(currentImage, &ubo, 0);
            DSb1p1.map(currentImage, &gubo, 2);

        }

    }
};

// This is the main: probably you do not need to touch this!
int main() {
    FinalProject app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
