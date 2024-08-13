// This has been adapted from the Vulkan tutorial

#include "modules/Starter.hpp"
#include "modules/TextMaker.hpp"


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


struct UniformBufferObject {
    alignas(16) glm::mat4 mvpMat;
    alignas(16) glm::mat4 mMat;
    alignas(16) glm::mat4 nMat;
    alignas(16) glm::vec4 color;
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
    Texture TskyBox, Tstars;
    DescriptorSet DSskyBox;


    Model MlargePlane;
    Texture TlargePlane;
    DescriptorSet DSPlane;

    Model Mbattleship;
    Texture Tbattleship;
    DescriptorSet DSBattleship;

    // Other application parameters
    int currScene = 0;
    int subpass = 0;
        
    glm::vec3 CamPos = glm::vec3(0.0, 0.1, 5.0);
    glm::mat4 ViewMatrix;
    float CamAlpha = 0.0f;
    float CamBeta = 0.0f;

    float Ar;
    
    // Here you set the main application parameters
    void setWindowParameters() {
        // window size, titile and initial background
        windowWidth = 800;
        windowHeight = 600;
        windowTitle = "FinalProject - Warships";
        windowResizable = GLFW_TRUE;
        initialBackgroundColor = {0.1f, 0.1f, 0.1f, 1.0f};
        
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
                    {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1}
                });
        DSLPlane.init(this, {
                    {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, sizeof(UniformBufferObject), 1},
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
        PskyBox.init(this, &VDskyBox, "shaders/SkyBoxVert.spv", "shaders/SkyBoxFrag.spv", {&DSLskyBox});
        PskyBox.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
                                    VK_CULL_MODE_BACK_BIT, false);
        
        PPlane.init(this, &VDPlane, "shaders/PhongVert.spv", "shaders/PhongFrag.spv", {&DSLPlane});
        PPlane.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
                                VK_CULL_MODE_BACK_BIT, false);

        PBattleship.init(this, &VDBattleship, "shaders/PhongVert.spv", "shaders/PhongFrag.spv", {&DSLBattleship});
        PBattleship.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
                                VK_CULL_MODE_BACK_BIT, false);


        // Create models
        MskyBox.init(this, &VDskyBox, "models/SkyBoxCube.obj", OBJ);
        MlargePlane.init(this, &VDPlane, "models/Water.obj", OBJ);
        Mbattleship.init(this, &VDBattleship, "models/Warships/Battleship.obj", OBJ);
        
        // Create the textures
        TskyBox.init(this, "textures/starmap_g4k.jpg");
        Tstars.init(this, "textures/constellation_figures.png");
        TlargePlane.init(this, "textures/water_9x9.png");
        Tbattleship.init(this, "textures/BattleshipTexture.jpeg");


        // Descriptor pool sizes
        // WARNING!!!!!!!!
        // Must be set before initializing the text and the scene
// **A10** Update the number of elements to correctly size the descriptor sets pool
        DPSZs.uniformBlocksInPool = 7;
        DPSZs.texturesInPool = 9;
        DPSZs.setsInPool = 5;

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
        DSskyBox.init(this, &DSLskyBox, {&TskyBox, &Tstars});
        DSPlane.init(this, &DSLPlane, { &TlargePlane });
        DSBattleship.init(this, &DSLBattleship, { &Tbattleship });

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
        DSBattleship.cleanup();
        DSGlobal.cleanup();

        txt.pipelinesAndDescriptorSetsCleanup();
    }

    // Here you destroy all the Models, Texture and Desc. Set Layouts you created!
    // All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
    // You also have to destroy the pipelines: since they need to be rebuilt, they have two different
    // methods: .cleanup() recreates them, while .destroy() delete them completely
    void localCleanup() {   
        TskyBox.cleanup();
        Tstars.cleanup();
        MskyBox.cleanup();

        TlargePlane.cleanup();
        MlargePlane.cleanup();

        Tbattleship.cleanup();
        Mbattleship.cleanup();
        
        // Cleanup descriptor set layouts
        DSLGlobal.cleanup();
        DSLskyBox.cleanup();
        DSLPlane.cleanup();
        DSLBattleship.cleanup();

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
                    static_cast<uint32_t>(MlargePlane.indices.size()), 1, 0, 0, 0);

        // Bind the pipeline for the battleship
        PBattleship.bind(commandBuffer);
        Mbattleship.bind(commandBuffer);
        DSBattleship.bind(commandBuffer, PBattleship, 0, currentImage);
        // Draw the battleship
        vkCmdDrawIndexed(commandBuffer, 
                    static_cast<uint32_t>(Mbattleship.indices.size()), 1, 0, 0, 0);


        txt.populateCommandBuffer(commandBuffer, currentImage, currScene);
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
        
        
        const float ROT_SPEED = glm::radians(120.0f);
        const float MOVE_SPEED = 5.0f; //If you want to move faster, increase this value
        
        
        // The Fly model update proc.
        CamAlpha = CamAlpha - ROT_SPEED * deltaT * r.y;
        CamBeta  = CamBeta  - ROT_SPEED * deltaT * r.x;
        CamBeta  =  CamBeta < glm::radians(-90.0f) ? glm::radians(-90.0f) :
                   (CamBeta > glm::radians( 90.0f) ? glm::radians( 90.0f) : CamBeta);

        glm::vec3 ux = glm::rotate(glm::mat4(1.0f), CamAlpha, glm::vec3(0,1,0)) * glm::vec4(1,0,0,1);
        glm::vec3 uz = glm::rotate(glm::mat4(1.0f), CamAlpha, glm::vec3(0,1,0)) * glm::vec4(0,0,1,1);
        CamPos = CamPos + MOVE_SPEED * m.x * ux * deltaT;
        CamPos = CamPos + MOVE_SPEED * m.y * glm::vec3(0,1,0) * deltaT;
        CamPos = CamPos + MOVE_SPEED * m.z * uz * deltaT;


        static float subpassTimer = 0.0;

        if(glfwGetKey(window, GLFW_KEY_SPACE)) {
            if(!debounce) {
                debounce = true;
                curDebounce = GLFW_KEY_SPACE;
                if(currScene != 1) {
                    currScene = (currScene+1) % outText.size();

                }
                if(currScene == 1) {
                    if(subpass >= 4) {
                        currScene = 0;
                    }
                }
                std::cout << "Scene : " << currScene << "\n";
                
                RebuildPipeline();
            }
        } else {
            if((curDebounce == GLFW_KEY_SPACE) && debounce) {
                debounce = false;
                curDebounce = 0;
            }
        }

        // Standard procedure to quit when the ESC key is pressed
        if(glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }


        if(glfwGetKey(window, GLFW_KEY_V)) {
            if(!debounce) {
                debounce = true;
                curDebounce = GLFW_KEY_V;

                printMat4("ViewMatrix  ", ViewMatrix);              
            }
        } else {
            if((curDebounce == GLFW_KEY_V) && debounce) {
                debounce = false;
                curDebounce = 0;
            }
        }

        if(glfwGetKey(window, GLFW_KEY_C)) {
            if(!debounce) {
                debounce = true;
                curDebounce = GLFW_KEY_C;
            }
        } else {
            if((curDebounce == GLFW_KEY_C) && debounce) {
                debounce = false;
                curDebounce = 0;
            }
        }

        if(glfwGetKey(window, GLFW_KEY_T)) {
            if(!debounce) {
                debounce = true;
                curDebounce = GLFW_KEY_T;
            }
        } else {
            if((curDebounce == GLFW_KEY_T) && debounce) {
                debounce = false;
                curDebounce = 0;
            }
        }

    
        if(currScene == 1) {
            switch(subpass) {
              case 0:
                    ViewMatrix   = glm::mat4(-0.0656882, -0.162777, 0.984474, 0, 0.0535786, 0.984606, 0.166374, 0, -0.996401, 0.0636756, -0.0559558, 0, 0.0649244, -0.531504, -3.26128, 1);
                break;
              case 1:
                    ViewMatrix   = glm::mat4(-0.312507, -0.442291, 0.840666, 0, 0.107287, 0.862893, 0.493868, 0, -0.943837, 0.24453, -0.222207, 0, -0.0157694, -0.186147, -1.54649, 1);
                break;
              case 2:
                    ViewMatrix   = glm::mat4(-0.992288, 0.00260993, -0.12393, 0, -0.0396232, 0.940648, 0.337063, 0, 0.117454, 0.339374, -0.93329, 0, 0.0335061, -0.0115242, -2.99662, 1);
                break;
              case 3:
                    ViewMatrix   = glm::mat4(0.0942192, -0.242781, 0.965495, 0, 0.560756, 0.814274, 0.150033, 0, -0.822603, 0.527272, 0.212861, 0, -0.567191, -0.254532, -1.79143, 1);
                break;
            }
        }
        
        if(currScene == 1) {
            subpassTimer += deltaT;
            if(subpassTimer > 4.0f) {
                subpassTimer = 0.0f;
                subpass++;
                std::cout << "Scene : " << currScene << " subpass: " << subpass << "\n";
                char buf[20];
                sprintf(buf, "FinalProject_%d.png", subpass);
                saveScreenshot(buf, currentImage);
                if(subpass == 4) {
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

        glm::mat4 Mv =  glm::rotate(glm::mat4(1.0), -CamBeta, glm::vec3(1,0,0)) *
                        glm::rotate(glm::mat4(1.0), -CamAlpha, glm::vec3(0,1,0)) *
                        glm::translate(glm::mat4(1.0), -CamPos);

        glm::mat4 ViewPrj =  M * Mv;
        glm::mat4 baseTr = glm::mat4(1.0f);                             

        // updates global uniforms
        // Global
        GlobalUniformBufferObject gubo{};
        gubo.lightDir = glm::vec3(cos(glm::radians(135.0f)), sin(glm::radians(135.0f)), 0.0f);
        gubo.lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        gubo.eyePos = glm::vec3(glm::inverse(ViewMatrix) * glm::vec4(0, 0, 0, 1));
        gubo.eyeDir = glm::vec4(0, 0, 0, 1) * ViewMatrix;
        DSGlobal.map(currentImage, &gubo, 0);


        UniformBufferObject ubo{};

        // Update uniforms for the plane
        ubo.mMat = glm::mat4(1.0f);
        ubo.mvpMat = ViewPrj * ubo.mMat;
        ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
        ubo.color = glm::vec4(1.0f);

        DSPlane.map(currentImage, &ubo, 0);
        DSPlane.map(currentImage, &gubo, 2);

        // Update uniforms for the battleship
		ubo.mMat = glm::mat4(1.0f);
		ubo.mvpMat = ViewPrj * ubo.mMat;
		ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
		ubo.color = glm::vec4(1.0f);

        DSBattleship.map(currentImage, &ubo, 0);
        DSBattleship.map(currentImage, &gubo, 2);

        
        skyBoxUniformBufferObject sbubo{};
        sbubo.mvpMat = M * glm::mat4(glm::mat3(Mv));
        DSskyBox.map(currentImage, &sbubo, 0);
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
