#define JSON_DIAGNOSTICS 1
#include "modules/Starter.hpp"
#include "modules/TextMaker.hpp"

std::vector<SingleText> outText = {
	{2, {"Playground Scene", "Press SPACE to exit"}, 0, 0}
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


// MAIN ! 
class FinalProject : public BaseProject {
	protected:
	
	// Descriptor Layouts ["classes" of what will be passed to the shaders]
	DescriptorSetLayout DSLPlane;

	// Vertex formats
	VertexDescriptor VDPlane;

	// Pipelines [Shader couples]
	Pipeline PPlane;

	// Models
	Model largePlaneModel;

	// Descriptor sets
	DescriptorSet DSPlane;
	
	// Textures
	Texture planeTexture;
	
	// Application parameters
	glm::mat4 Wm;
	glm::vec4 MCol;
	
	TextMaker txt;
	


	/**
	 * Camera parameters presi dall'assignment 2
	 */
	// Camera parameters
	glm::vec3 CamPos = glm::vec3(0.0, 1.5, 7.0);
	float CamAlpha = 0.0f;
	float CamBeta = 0.0f;
	float Ar;
	bool WireFrame = false;

	/**
	 * Funzione per settare dimensione, titolo e Aspect Ratio della finestra di gioco
	 */
	void setWindowParameters() {
	    // window size, title and initial background
	    windowWidth = 800;
	    windowHeight = 600;
	    windowTitle = "FinalProject - BattleShip";
	    windowResizable = GLFW_TRUE;
	    initialBackgroundColor = {0.0f, 0.85f, 1.0f, 1.0f};

		/*
		Allocazione efficiente delle risorse: Specificare le dimensioni dei pool aiuta a 
		pre-allocare le risorse in modo efficiente, evitando rallentamenti durante il runtime 
		dovuti alla necessità di allocare dinamicamente nuove risorse. 
		
		Identifica e conta il numero di modelli, texture e altre risorse necessarie per la tua scena 
		applicazione. Ad esempio, numAssets potrebbe rappresentare il numero di 
		modelli principali e numAssets2 potrebbe rappresentare il numero di modelli aggiuntivi.
		*/
		uniformBlocksInPool = 5;
        texturesInPool = 4;
        setsInPool = 5;

	    Ar = 4.0f / 3.0f;
	}
	
	/**
	 * Funzione per aggiustare l'Aspect Ratio quando modifichiamo le dimensioni della finestra
	 */
	void onWindowResize(int w, int h) {
		std::cout << "Window resized to: " << w << " x " << h << "\n";
		Ar = (float)w / (float)h;
	}
	
	void localInit() {
		// Descriptor Layouts [what will be passed to the shaders]
		DSLPlane.init(this, {
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
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

		// Pipelines [Shader couples]
		PPlane.init(this, &VDPlane, "shaders/PhongVert.spv", "shaders/PhongFrag.spv", {&DSLPlane});
		PPlane.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
 							    VK_CULL_MODE_BACK_BIT, false);

		// Models, textures and Descriptors (values assigned to the uniforms)
		largePlaneModel.init(this, &VDPlane, "models/LargePlane.obj", OBJ);

		// Initialize the world matrix and color for the large plane model
		Wm = glm::mat4(1.0f);
		MCol = glm::vec4(1.0f);

		// Create the textures
		planeTexture.init(this, "textures/water.png");
		
		// Initialize text maker
		txt.init(this, &outText);
	}
	
	void pipelinesAndDescriptorSetsInit() {
		// Create pipeline
		PPlane.create();

		// Define the data set
		DSPlane.init(this, &DSLPlane, { 
			{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, 
			{1, TEXTURE, 0, &planeTexture},
			{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
		});
		
		txt.pipelinesAndDescriptorSetsInit();
	}

	void pipelinesAndDescriptorSetsCleanup() {
		// Cleanup pipeline
		PPlane.cleanup();

		// Cleanup descriptor set
		DSPlane.cleanup();
		
		txt.pipelinesAndDescriptorSetsCleanup();
	}

	void localCleanup() {	
		// Cleanup texture
		planeTexture.cleanup();
		
		// Cleanup model
		largePlaneModel.cleanup();
		
		// Cleanup descriptor set layout
		DSLPlane.cleanup();
		
		// Destroy pipeline
		PPlane.destroy();
		
		txt.localCleanup();
	}
	
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
		// Bind the pipeline
		PPlane.bind(commandBuffer);
		
		// Bind the model
		largePlaneModel.bind(commandBuffer);
		DSPlane.bind(commandBuffer, PPlane, 0, currentImage);
					
		// Draw the model
		vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(largePlaneModel.indices.size()), 1, 0, 0, 0);

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
		ubo.mMat = Wm;
		ubo.mvpMat = ViewPrj * ubo.mMat;
		ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
		ubo.color = MCol;

		// Update global uniforms
		GlobalUniformBufferObject gubo{};
		gubo.lightDir = glm::vec3(cos(glm::radians(135.0f)), sin(glm::radians(135.0f)), 0.0f);
		gubo.lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		gubo.eyePos = CamPos;

		// Map the uniforms
		DSPlane.map(currentImage, &ubo, sizeof(ubo),0);
		DSPlane.map(currentImage, &gubo, sizeof(gubo), 2);
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
