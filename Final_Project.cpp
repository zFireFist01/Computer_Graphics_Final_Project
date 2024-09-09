// This has been adapted from the Vulkan tutorial

#include "modules/Starter.hpp"
#include "modules/TextMaker.hpp"

#define NPLANE 2

std::vector<SingleText> outText = {
    {2, {"Player 0.", "Select X_position of boat 0 [0-8]","",""}, 0, 0},
    {2, {"Player 0.", "Select Y_position of boat 0 [0-8]","",""}, 0, 0},
    {2, {"Player 0.", "Select X_position of boat 1 [0-8]","",""}, 0, 0},
    {2, {"Player 0.", "Select Y_position of boat 1 [0-8]","",""}, 0, 0},
    {2, {"Player 1.", "Select X_position of boat 0 [0-8]","",""}, 0, 0},
    {2, {"Player 1.", "Select Y_position of boat 0 [0-8]","",""}, 0, 0},
    {2, {"Player 1.", "Select X_position of boat 1 [0-8]","",""}, 0, 0},
    {2, {"Player 1.", "Select Y_position of boat 1 [0-8]","",""}, 0, 0},
    {1, {"Player 1 select X attack [0-8]", "", "",""}, 0, 0},
    {1, {"Player 1 select Y attack [0-8]", "", "",""}, 0, 0},
    {1, {"Player 2 select X attack [0-8]", "", "",""}, 0, 0},
    {1, {"Player 2 select Y attack [0-8]", "", "",""}, 0, 0}, 
    {1, {"Player 2 select X attack [0-8]", "", "",""}, 0, 0},
    {1, {"", "", "", ""}, 0, 0}
};

// The uniform buffer object used in this example
struct GlobalUniformBufferObject {
    glm::vec4 lightDir[3];
    glm::vec4 lightPos;
    glm::vec4 lightColor[3];
    glm::vec4 eyePos;
};

enum GameState {
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
    glm::mat4 mvpMat;
    glm::mat4 mMat;
    glm::mat4 nMat;
};

struct PlaneUniformBufferObject {
    glm::mat4 mvpMat[NPLANE];
    glm::mat4 mMat[NPLANE];
    glm::mat4 nMat[NPLANE];
};

struct skyBoxUniformBufferObject {
    glm::mat4 mvpMat;
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

// MAIN ! 
class FinalProject : public BaseProject {
protected:

    // Descriptor Layouts ["classes" of what will be passed to the shaders]
    DescriptorSetLayout DSLGlobal;  // For Global values

    DescriptorSetLayout DSLskyBox;  // For skyBox
    DescriptorSetLayout DSLPlane;   // For the plane
    DescriptorSetLayout DSLVerticalPlane;  // For the vertical plane
    DescriptorSetLayout DSLBattleship;  // For the battleship

    // Vertex formats
    VertexDescriptor VDskyBox;
    VertexDescriptor VDPlane;
    VertexDescriptor VDBattleship;

    // Pipelines [Shader couples]
    Pipeline PskyBox;
    Pipeline PPlane;
    Pipeline PVerticalPlane;
    Pipeline PBattleship;
    Pipeline PExplosionSphere;

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

    Model MverticalPlane;  // Modello per il piano verticale
    Texture TverticalPlane;  // Texture per il piano verticale
    DescriptorSet DSVerticalPlane;

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


    GlobalUniformBufferObject gubo;
    PlaneUniformBufferObject pubo;
    PlaneUniformBufferObject Vpubo;

    // Other application parameters
    int currScene = 0;
    int subpass = 0;
    int currPlayer = 0;

    glm::vec3 CamPos;
    glm::mat4 ViewMatrix;
    float CamAlpha;
    float CamBeta;
    glm::vec3 forward;
    float Ar;
    const float ROT_SPEED = glm::radians(80.0f);
    const float MOVE_SPEED = 30.0f; //If you want to move faster, increase this value

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
    bool B0P0Animated = false;
    bool B1P0Animated = false;
    bool B0P1Animated = false;
    bool B1P1Animated = false;

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
    float x;
    float y;
    float z;
    glm::vec3 velocity;
    //Explosion variables
    glm::vec3 explosionCenter = glm::vec3(0.0f, -100.0f, 0.0f);
    float explosionRadius = 0.0f;
    float explosionMaxRadius = 1.0f;
    float explosionTime = 0.0f;
    float explosionDuration = 3.0f;
    bool isExplosionVisible = false;

    // Creiamo una matrice S x T di glm::mat4
    glm::mat4 matrix[9][9];
    glm::mat4 matrixB[9][9];

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
        DSLVerticalPlane.init(this, {
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
                {0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, norm),
                    sizeof(glm::vec3), NORMAL},
                {0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, UV),
                    sizeof(glm::vec2), UV}
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

        PPlane.init(this, &VDPlane, "shaders/PlaneVert.spv", "shaders/PlaneFrag.spv", { &DSLGlobal, &DSLPlane });
        PPlane.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_BACK_BIT, false);

        PVerticalPlane.init(this, &VDPlane, "shaders/PlaneVert.spv", "shaders/PlaneFrag.spv", {&DSLGlobal, &DSLVerticalPlane });
        PVerticalPlane.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_BACK_BIT, false);

        PBattleship.init(this, &VDBattleship, "shaders/BattleshipVert.spv", "shaders/BattleshipFrag.spv", { &DSLBattleship });
        PBattleship.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_BACK_BIT, false);

        PExplosionSphere.init(this, &VDBattleship, "shaders/BattleshipVert.spv", "shaders/BattleshipFrag.spv", { &DSLBattleship });
        PExplosionSphere.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_BACK_BIT, false);

        // Create models
        MskyBox.init(this, &VDskyBox, "models/SkyBoxCube.obj", OBJ);
        MlargePlane.init(this, &VDPlane, "models/Water.obj", OBJ);
        Mb0p0.init(this, &VDBattleship, "models/Warships/Battleship.obj", OBJ);
        Mb1p0.init(this, &VDBattleship, "models/Warships/Battleship.obj", OBJ);
        Mb0p1.init(this, &VDBattleship, "models/Warships/Battleship.obj", OBJ);
        Mb1p1.init(this, &VDBattleship, "models/Warships/Battleship.obj", OBJ);
        Mmissile.init(this, &VDBattleship, "models/Missile/missile3.obj", OBJ);
        MverticalPlane.init(this, &VDPlane, "models/VerticalPlane.obj", OBJ);  // Inizializza il modello del piano verticale
        MExplosionSphere.init(this, &VDBattleship, "models/Sphere.obj", OBJ);

        // Create the textures
        TskyBox.init(this, "textures/cielo.jpg");
        TlargePlane.init(this, "textures/water_cropped_grid.jpg");
        Tbattleship.init(this, "textures/Metal.jpg");
        Tmissile.init(this, "textures/missile_texture.jpg");
        TverticalPlane.init(this, "textures/texvertplaneA.jpg");  // Inizializza la texture del piano verticale (sostituisci con il percorso corretto)
        TExplosionSphere.init(this, "textures/explosion_texture.jpg");

        // Descriptor pool sizes
        // WARNING!!!!!!!!
        // Must be set before initializing the text and the scene
        DPSZs.uniformBlocksInPool = 30;
        DPSZs.texturesInPool = 30;
        DPSZs.setsInPool = 30;

        std::cout << "Initializing text\n";
        txt.init(this, &outText);

        std::cout << "Initialization completed!\n";
        std::cout << "Uniform Blocks in the Pool  : " << DPSZs.uniformBlocksInPool << "\n";
        std::cout << "Textures in the Pool        : " << DPSZs.texturesInPool << "\n";
        std::cout << "Descriptor Sets in the Pool : " << DPSZs.setsInPool << "\n";

        CamPos = glm::vec3(0.0f, 50.0f, 100.0f);
        ViewMatrix = glm::lookAt(CamPos, glm::vec3(0.0f, 50.0f, -300.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        forward = glm::normalize(glm::vec3(-ViewMatrix[2][0], -ViewMatrix[2][1], -ViewMatrix[2][2]));
        CamAlpha = glm::atan(forward.x, forward.z);
        CamBeta = glm::asin(forward.y);

        //glm::mat4(1.0f) traslata di 1.5 rispetto all'asse x è il centro della scacchiera
        //Ogni elemento della colonna 5 ha una traslazione x di 1.5f
        //Ogni elemento si muove a multipli di 21.3f rispetto alla z
        // Ora costruiamo la scacchiera con le rispettive matrici
        for (int i = 0; i < 9; ++i) {
            for (int j = 0; j < 9; ++j) {
                matrix[i][j] = glm::translate(glm::mat4(1.0f), glm::vec3(22.0f * (j - 4), 0.0f, 22.0f * (i - 4)));
                matrixB[i][j] = glm::translate(glm::rotate(matrix[i][j], glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec3(0.0f, 0.0f, 193.0f));
            }
        }
    }

    // Here you create your pipelines and Descriptor Sets!
    void pipelinesAndDescriptorSetsInit() {
        // This creates a new pipeline (with the current surface), using its shaders
        PskyBox.create();
        PPlane.create();
        PVerticalPlane.create();
        PBattleship.create();
        PExplosionSphere.create();

        // Here you define the data set
        DSskyBox.init(this, &DSLskyBox, { &TskyBox });
        DSPlane.init(this, &DSLPlane, { &TlargePlane });
        DSb0p0.init(this, &DSLBattleship, { &Tbattleship });
        DSb1p0.init(this, &DSLBattleship, { &Tbattleship });
        DSb0p1.init(this, &DSLBattleship, { &Tbattleship });
        DSb1p1.init(this, &DSLBattleship, { &Tbattleship });
        DSmissile.init(this, &DSLBattleship, { &Tmissile });
        DSVerticalPlane.init(this, &DSLVerticalPlane, { &TverticalPlane });  // Inizializza il descriptor set per il piano verticale con la sua texture
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
        PVerticalPlane.cleanup();
        PBattleship.cleanup();
        PExplosionSphere.cleanup();

        DSskyBox.cleanup();
        DSPlane.cleanup();
        DSb0p0.cleanup();
        DSb1p0.cleanup();
        DSb0p1.cleanup();
        DSb1p1.cleanup();
        DSmissile.cleanup();
        DSGlobal.cleanup();
        DSVerticalPlane.cleanup();
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
        MverticalPlane.cleanup();

        Tmissile.cleanup();
        Mmissile.cleanup();

        MExplosionSphere.cleanup();
        TExplosionSphere.cleanup();

        // Cleanup descriptor set layouts
        DSLGlobal.cleanup();
        DSLskyBox.cleanup();
        DSLPlane.cleanup();
        DSLVerticalPlane.cleanup();
        DSLBattleship.cleanup();

        // Destroies the pipelines
        PskyBox.destroy();
        PPlane.destroy();
        PVerticalPlane.destroy();
        PBattleship.destroy();
        PExplosionSphere.destroy();

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
        DSGlobal.bind(commandBuffer, PPlane, 0, currentImage);
        DSPlane.bind(commandBuffer, PPlane, 1, currentImage);
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
        PVerticalPlane.bind(commandBuffer);
        MverticalPlane.bind(commandBuffer);
        DSGlobal.bind(commandBuffer, PVerticalPlane, 0, currentImage);
        DSVerticalPlane.bind(commandBuffer, PVerticalPlane, 1, currentImage);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(MverticalPlane.indices.size()), NPLANE, 0, 0, 0);
        
        PExplosionSphere.bind(commandBuffer);
        MExplosionSphere.bind(commandBuffer);
        DSExplosionSphere.bind(commandBuffer, PExplosionSphere, 0, currentImage);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(MExplosionSphere.indices.size()), 1, 0, 0, 0);

        txt.populateCommandBuffer(commandBuffer, currentImage, currScene);

        staticUniformBuffer(currentImage);
    }


    void staticUniformBuffer(uint32_t currentImage) {
        //Point Light
        gubo.lightDir[0]= glm::vec4(cos(glm::radians(90.0f)), sin(glm::radians(90.0f)), 0.0f, 1.0f);
        gubo.lightColor[0] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        gubo.lightPos = glm::vec4(0.0f, 4.0f,  -100.0f, 1.0f);

        //Direct Light 1
        gubo.lightDir[1] = glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
        gubo.lightColor[1] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); 

        //Direct Light 2    
        gubo.lightDir[2] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
        gubo.lightColor[2] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

        gubo.eyePos = glm::vec4(100.0f, 120.0f, 200.0f, 1.0f);


        // Update uniforms for the plane
        pubo.mMat[0] = glm::mat4(1.0f);
        pubo.nMat[0] = glm::inverse(glm::transpose(pubo.mMat[0]));

        pubo.mMat[1] = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -195.0f));
        pubo.nMat[1] = glm::inverse(glm::transpose(pubo.mMat[1]));

        // Update uniforms for the vertical plane
        Vpubo.mMat[0] = glm::mat4(1.0f) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        Vpubo.nMat[0] = glm::inverse(glm::transpose(Vpubo.mMat[0]));

        Vpubo.mMat[1] = glm::translate(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec3(-194.0f, 0.0f, 0.0f));
        Vpubo.nMat[1] = glm::inverse(glm::transpose(Vpubo.mMat[1]));
    }

    // Here is where you update the uniforms.
    // Very likely this will be where you will be writing the logic of your application.
    void updateUniformBuffer(uint32_t currentImage) {
        static bool debounce = false;
        static int curDebounce = 0;

        float deltaT;
        glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
        bool fire = false;
        getSixAxis(deltaT, m, r, fire);

        static float subpassTimer = 0.0;

        // Standard procedure to quit when the ESC key is pressed
        if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }

        if (currentState == WAITING_ATTACK_X || currentState == WAITING_ATTACK_Y || currentState == WAITING_BOAT_X || currentState == WAITING_BOAT_Y) {
            // The Fly model update proc.
            CamAlpha = CamAlpha - ROT_SPEED * deltaT * r.y;
            CamBeta = CamBeta - ROT_SPEED * deltaT * r.x;
            CamBeta = CamBeta < glm::radians(-90.0f) ? glm::radians(-90.0f) :
                (CamBeta > glm::radians(90.0f) ? glm::radians(90.0f) : CamBeta);

            glm::vec3 ux = glm::rotate(glm::mat4(1.0f), CamAlpha, glm::vec3(0, 1, 0)) * glm::vec4(1, 0, 0, 1);
            glm::vec3 uz = glm::rotate(glm::mat4(1.0f), CamAlpha, glm::vec3(0, 1, 0)) * glm::vec4(0, 0, 1, 1);
            CamPos = CamPos - MOVE_SPEED * m.x * ux * deltaT;
            CamPos = CamPos + MOVE_SPEED * m.y * glm::vec3(0, 1, 0) * deltaT;
            CamPos = CamPos - MOVE_SPEED * m.z * uz * deltaT;
            forward = glm::vec3(
                cos(CamBeta) * sin(CamAlpha),
                sin(CamBeta),
                cos(CamBeta) * cos(CamAlpha)
            );

            // Use lookAt to generate the view matrix
            ViewMatrix = glm::lookAt(CamPos, CamPos + forward, glm::vec3(0.0f, 1.0f, 0.0f));
        }

        // Here is where you actually update your uniforms
        glm::mat4 M = glm::perspective(glm::radians(45.0f), Ar, 0.1f, 1000.0f); // Projection matrix; If you want to see further icrease the last parameter
        M[1][1] *= -1;

        glm::mat4 Mv = ViewMatrix;
        glm::mat4 ViewPrj = M * Mv;
        glm::mat4 baseTr = glm::mat4(1.0f);


        DSGlobal.map(currentImage, &gubo, 0);

        // Update uniforms for the plane
        pubo.mvpMat[0] = ViewPrj * pubo.mMat[0];
        pubo.mvpMat[1] = ViewPrj * pubo.mMat[1];

        DSPlane.map(currentImage, &pubo, 0);

        skyBoxUniformBufferObject sbubo{};
        sbubo.mvpMat = M * glm::mat4(glm::mat3(Mv));
        DSskyBox.map(currentImage, &sbubo, 0);

        //For the vertical plane
        Vpubo.mvpMat[0] = ViewPrj * Vpubo.mMat[0];
        Vpubo.mvpMat[1] = ViewPrj * Vpubo.mMat[1];

        DSVerticalPlane.map(currentImage, &Vpubo, 0);

        UniformBufferObject uboMissile{};
        UniformBufferObject uboExplosion{};
        //FSA che gestisce le fasi di gioco
        switch (currentState) {
            case WAITING_BOAT_X: {
                if (currPlayer == 0) {
                    if (B0P0_x == -1) {
                        currScene = 0;
                        RebuildPipeline();
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
                        currScene = 2;
                        RebuildPipeline();
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
                        currScene = 4;
                        RebuildPipeline();
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
                        currScene = 6;
                        RebuildPipeline();
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
                        currScene = 1;
                        RebuildPipeline();
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
                        currScene = 3;
                        RebuildPipeline();
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
                        currScene = 5;
                        RebuildPipeline();
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
                        currScene = 7;
                        RebuildPipeline();
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
                currScene = 12;
                RebuildPipeline();
                UniformBufferObject ubo{};
                // Update uniforms for the battleship
                ubo.mMat = matrix[B0P0_x][B0P0_y];
                ubo.mvpMat = ViewPrj * ubo.mMat;
                ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));

                DSb0p0.map(currentImage, &ubo, 0);

                // Update uniforms for the battleship
                ubo.mMat = matrix[B1P0_x][B1P0_y];
                ubo.mvpMat = ViewPrj * ubo.mMat;
                ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));

                DSb1p0.map(currentImage, &ubo, 0);

                // TODO: mancano le battelship del giocatore 1 e vanno aggiunte mappandole sulla seconda tavola
                ubo.mMat = matrixB[B0P1_x][B0P1_y];
                ubo.mvpMat = ViewPrj * ubo.mMat;
                ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));

                DSb0p1.map(currentImage, &ubo, 0);

                ubo.mMat = matrixB[B1P1_x][B1P1_y];
                ubo.mvpMat = ViewPrj * ubo.mMat;
                ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));

                DSb1p1.map(currentImage, &ubo, 0);

                currentState = WAITING_ATTACK_X;
                boatVisible = true;
                break;
            }

            case WAITING_ATTACK_X: {
                if (currPlayer == 0) {
                    currScene = 8;
                }
                else {
                    currScene = 10;
                }
                RebuildPipeline();
                
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
                if (currPlayer == 0) {
                    currScene = 9;
                }
                else {
                    currScene = 11;
                }
                RebuildPipeline();

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
                currScene = 12;
                RebuildPipeline();

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
                    h = length(missileEndPos - missileStartPos) * 0.8;
                    currentState = ANIMATING_MISSILE;
                }
                isMissileVisible = true;
                missileStartPos = glm::vec3(0.0f, 0.0f, 100.0f + (-400.0f * currPlayer));
                break;
            }

            case ANIMATING_MISSILE: {
                // Aggiorna la posizione del missile
                missileTime += deltaT / totalTime; // Aggiorna il tempo normalizzato (0 -> 1)

                // Clamp del tempo per assicurarsi che rimanga nell'intervallo [0, 1]
                missileTime = glm::clamp(missileTime, 0.0f, 1.0f);

                // Calcola la nuova posizione del missile
                x = missileStartPos.x + missileTime * (missileEndPos.x - missileStartPos.x);
                y = missileStartPos.y + 4 * h * missileTime * (1.0f - missileTime);  // Vertical component for the parabolic arc
                z = missileStartPos.z + missileTime * (missileEndPos.z - missileStartPos.z);  // Horizontal direction

                missilePos = glm::vec3(x, y, z);

                glm::vec3 velocity = glm::normalize(glm::vec3(
                    missileEndPos.x - missileStartPos.x,
                    4 * h * (1.0f - 2.0f * missileTime),  // Derivative of the vertical parabolic function
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

                    isMissileVisible = false;
                    currPlayer = (currPlayer + 1) % 2;
                    std::cout << "currentPlayer = " << currPlayer << '\n';
                    currentState = WAITING_ATTACK_X;  // Torna allo stato di attesa per nuove coordinate
                    if (currPlayer == 1) {
                        CamPos = glm::vec3(0.0f, 50.0f, -300.0f);  // Posizione dietro al vertical plane
                        ViewMatrix = glm::lookAt(CamPos, glm::vec3(0.0f, 50.0f, 100.0f), glm::vec3(0.0f, 1.0f, 0.0f));  // TODO: non guarda verso il piano verticale
                        forward = glm::normalize(glm::vec3(-ViewMatrix[2][0], -ViewMatrix[2][1], -ViewMatrix[2][2]));
                        CamBeta = glm::asin(forward.y);
                        CamAlpha = glm::atan(forward.x, forward.z);
                        if (targetX == B0P1_x && targetY == B0P1_y && B0P1Alive) {
                            B0P1Alive = false;
                            B0P1Animated = true;
                            isExplosionVisible = true;
                            explosionCenter = missileEndPos;
                            explosionRadius = 0.0f;
                            explosionTime = 0.0f;
                            currentState = ANIMATING_EXPLOSION;
                        }
                        else if (targetX == B1P1_x && targetY == B1P1_y && B1P1Alive) {
                            B1P1Alive = false;
                            B1P1Animated = true;
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
                        forward = glm::normalize(glm::vec3(-ViewMatrix[2][0], -ViewMatrix[2][1], -ViewMatrix[2][2]));
                        CamBeta = glm::asin(forward.y);
                        CamAlpha = glm::atan(forward.x, forward.z);
                        if (targetX == B0P0_x && targetY == B0P0_y && B0P0Alive) {
                            B0P0Alive = false;
                            B0P0Animated = true;
                            isExplosionVisible = true;
                            explosionCenter = missileEndPos;
                            explosionRadius = 0.0f;
                            explosionTime = 0.0f;
                            currentState = ANIMATING_EXPLOSION;
                        }
                        else if (targetX == B1P0_x && targetY == B1P0_y && B1P0Alive) {
                            B1P0Alive = false;
                            B1P0Animated = true;
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
                    CamPos = missilePos + glm::vec3(0.0f, 8.0f, 0.0f);
                    forward = missileEndPos;// glm::normalize(velocity);
                    ViewMatrix = glm::lookAt(CamPos, forward, up);

                    // Update the uniform buffer for the missile
                    uboMissile.mMat = modelMatrix;  // Apply the transformation
                    uboMissile.mvpMat = ViewPrj * uboMissile.mMat;
                    uboMissile.nMat = glm::inverse(glm::transpose(uboMissile.mMat));

                    DSmissile.map(currentImage, &uboMissile, 0);
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
                    B0P0Animated = false;
                    B0P1Animated = false;
                    B1P0Animated = false;
                    B1P1Animated = false;

                    if (currPlayer == 1) {
                        CamPos = glm::vec3(0.0f, 50.0f, -300.0f);  // Posizione dietro al vertical plane
                        ViewMatrix = glm::lookAt(CamPos, glm::vec3(0.0f, 50.0f, 100.0f), glm::vec3(0.0f, 1.0f, 0.0f));  // TODO: non guarda verso il piano verticale
                        forward = glm::normalize(glm::vec3(-ViewMatrix[2][0], -ViewMatrix[2][1], -ViewMatrix[2][2]));
                        CamBeta = glm::asin(forward.y);
                        CamAlpha = glm::atan(forward.x, forward.z);
                    }
                    else {
                        CamPos = glm::vec3(0.0f, 50.0f, 100.0f);
                        ViewMatrix = glm::lookAt(CamPos, glm::vec3(0.0f, 50.0f, -300.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // TODO: non guarda verso il piano verticale
                        forward = glm::normalize(glm::vec3(-ViewMatrix[2][0], -ViewMatrix[2][1], -ViewMatrix[2][2]));
                        CamBeta = glm::asin(forward.y);
                        CamAlpha = glm::atan(forward.x, forward.z);
                    }
                }

                // Aggiorna la matrice di trasformazione della sfera dell'esplosione
                glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), explosionCenter) *
                    glm::scale(glm::mat4(1.0f), glm::vec3(explosionRadius));

                uboExplosion.mMat = modelMatrix;
                uboExplosion.mvpMat = ViewPrj * uboExplosion.mMat;
                uboExplosion.nMat = glm::inverse(glm::transpose(uboExplosion.mMat));

                DSExplosionSphere.map(currentImage, &uboExplosion, 0);
            }
        }

        // renderizza fuori campo il missile nel caso non sia visibile
        if (!isMissileVisible) {
            DSmissile.map(currentImage, &uboMissile, 0);
        }

        if (!isExplosionVisible) {
            DSExplosionSphere.map(currentImage, &uboExplosion, 0);
        }

        UniformBufferObject ubo{};
        if (boatVisible) {
            // Update uniforms for the battleship
            if (B0P0Alive || B0P0Animated) {
                ubo.mMat = matrix[B0P0_x][B0P0_y];
            }
            else {
                ubo.mMat = glm::translate(matrix[B0P0_x][B0P0_y], glm::vec3(0.0f, -100.0f, 0.0f));
            }

            ubo.mvpMat = ViewPrj * ubo.mMat;
            ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));

            DSb0p0.map(currentImage, &ubo, 0);

            // Update uniforms for the battleship
            if (B1P0Alive || B1P0Animated) {
                ubo.mMat = matrix[B1P0_x][B1P0_y];
            }
            else {
                ubo.mMat = glm::translate(matrix[B1P0_x][B1P0_y], glm::vec3(0.0f, -100.0f, 0.0f));
            }

            ubo.mvpMat = ViewPrj * ubo.mMat;
            ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));

            DSb1p0.map(currentImage, &ubo, 0);

            // TODO: mancano le battelship del giocatore 1 e vanno aggiunte mappandole sulla seconda tavola
            if (B0P1Alive || B0P1Animated) {
                ubo.mMat = matrixB[B0P1_x][B0P1_y];
            }
            else {
                ubo.mMat = glm::translate(matrixB[B0P1_x][B0P1_y], glm::vec3(0.0f, -100.0f, 0.0f));
            }

            ubo.mvpMat = ViewPrj * ubo.mMat;
            ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));

            DSb0p1.map(currentImage, &ubo, 0);

            if (B1P1Alive || B1P1Animated) {
                ubo.mMat = matrixB[B1P1_x][B1P1_y];
            }
            else {
                ubo.mMat = glm::translate(matrixB[B1P1_x][B1P1_y], glm::vec3(0.0f, -100.0f, 0.0f));
            }
            ubo.mvpMat = ViewPrj * ubo.mMat;
            ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));

            DSb1p1.map(currentImage, &ubo, 0);
        }
        else { // posizioni iniziali causali - TODO: inserire pure quelle del giocatore 2
            ubo.mMat = matrix[1][3];
            ubo.mvpMat = ViewPrj * ubo.mMat;
            ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));

            DSb0p0.map(currentImage, &ubo, 0);

            // Update uniforms for the battleship
            ubo.mMat = matrix[8][6];
            ubo.mvpMat = ViewPrj * ubo.mMat;
            ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));

            DSb1p0.map(currentImage, &ubo, 0);

            // TODO: mancano le battelship del giocatore 1 e vanno aggiunte mappandole sulla seconda tavola
            ubo.mMat = matrixB[0][2];
            ubo.mvpMat = ViewPrj * ubo.mMat;
            ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));

            DSb0p1.map(currentImage, &ubo, 0);

            ubo.mMat = matrixB[7][5];
            ubo.mvpMat = ViewPrj * ubo.mMat;
            ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));

            DSb1p1.map(currentImage, &ubo, 0);
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
