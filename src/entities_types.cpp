#include"entities_types.h"
//---------- Base entity types ----------

//----- Camera3D -----

//Variables
Camera3d* Camera3d::current = nullptr;

//Functions
glm::mat4 Camera3d::create_perspective_matrix(float fov, float aspectRatio, float near, float far) 
{
    return glm::perspective(glm::radians(fov),aspectRatio,near,far);
}
Camera3d::Camera3d(float FOV, std::string name)
{
    this->name = name;
    Camera3d::current = this;
    this->FOV = FOV;
    perspective_matrix_cache = glm::mat4();
    display_property("FOV",&this->FOV,PROPERTY_FLOAT);
}
void Camera3d::set_current()
{
    Camera3d::current = this;
}
glm::mat4 Camera3d::get_perspective_matrix(int w, int h)
{
    return create_perspective_matrix(FOV,(float)w/(float)h,0.01f,500.0f);
}
glm::mat4 Camera3d::get_view_matrix()
{
    //return glm::translate(glm::toMat4(glm::quat(rotation)),-position);
    return glm::lookAt(position,position + glm::vec3(0.0f,0.0f,-1.0f)*glm::quat(rotation),glm::vec3(0.0f,1.0f,0.0f));
}
void Camera3d::draw(float delta)
{
    glm::vec3 forward = glm::vec3(0, 0, -1.0f)*glm::quat(rotation);
    debug_color = glm::vec3(1.0f,1.0f,1.0f);
    debug_begin_frame();
    debug_draw_line(position,position + forward);
    debug_end_frame();
}

//----- MeshRenderer -----

//Functions
MeshRenderer::MeshRenderer(aiMesh* mesh, Material* material, std::string name)
{
    this->name = name;
    this->mesh = mesh;
    this->color = glm::vec4(1.0,1.0,1.0,1.0);

    //Get vertices and normals
    std::vector<float> verts = std::vector<float>();
    std::vector<float> normals = std::vector<float>();
    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        verts.push_back(mesh->mVertices[i].x);
        verts.push_back(mesh->mVertices[i].y);
        verts.push_back(mesh->mVertices[i].z);
        normals.push_back(mesh->mNormals[i].x);
        normals.push_back(mesh->mNormals[i].y);
        normals.push_back(mesh->mNormals[i].z);
    }

    //Get indices
    std::vector<unsigned int> indie = std::vector<unsigned int>();
    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        indie.push_back(mesh->mFaces[i].mIndices[0]);
        indie.push_back(mesh->mFaces[i].mIndices[1]);
        indie.push_back(mesh->mFaces[i].mIndices[2]);
    }
    this->IndicesCount = indie.size();
    this->VerticesCount = verts.size();
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    //Bind VAO, VBO, and EBO
    glBindVertexArray(VAO);
    //set VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(float), verts.data(), GL_STATIC_DRAW);
    //set EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indie.size() * sizeof(unsigned int), indie.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal vectors attribute
    GLuint normalbuffer;
    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), &normals[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0); 

    //Set material
    this->material = material;

    //Display properies
    display_property("Mesh",&mesh_asset_path,PROPERTY_ASSET);
    display_property("Material",&material_asset_path,PROPERTY_ASSET);
    display_property("Color",&color,PROPERTY_COLOR);
}
void MeshRenderer::draw(float delta)
{
    //Use shader
    glUseProgram(material->GL_shader_id);
    glBindAttribLocation(material->GL_shader_id, 0, "aPos");
    glBindAttribLocation(material->GL_shader_id, 1, "aNor");

    //Assign shader properties
    glm::mat4 projection = Camera3d::current->get_perspective_matrix(current_framebuffer_width,currrent_framebuffer_height);
    set_shader_matrix4_value(material->GL_shader_id, "Projection", projection);
    set_shader_matrix4_value(material->GL_shader_id, "View", Camera3d::current->get_view_matrix());
    set_shader_matrix4_value(material->GL_shader_id, "Model", get_model_matrix());
    set_shader_vector4_value(material->GL_shader_id, "Color", color);
    
    //Draw
    glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
    glDrawElements(GL_TRIANGLES, IndicesCount, GL_UNSIGNED_INT, 0);
}

//ScriptableEntity


std::string input_file_string(const std::string& file_path) 
{
    // Check if the file exists and is a regular file
    if (!std::filesystem::exists(file_path) || !std::filesystem::is_regular_file(file_path)) {
        return ""; // Return an empty string to indicate an error
    }

    std::ifstream input_file(file_path);

    if (!input_file.is_open()) {
        return ""; // Return an empty string to indicate an error
    }

    std::string file_contents(
        (std::istreambuf_iterator<char>(input_file)),
        (std::istreambuf_iterator<char>())
    );

    input_file.close();

    return file_contents;
}

//Functions
ScriptableEntity::ScriptableEntity(std::string name,std::string script_src, std::string script_string)
{
    this->name = name;
    this->script_src = script_src;
    this->script_string = script_string;
    this->opened = false;
    
    display_property(script_src,&this->script_src,PROPERTY_SCRIPT);
    
    my_script_state = tea_open();
    
    //Add modules
    TeaModule_add_debug(my_script_state);
    TeaModule_add_input(my_script_state);
    TeaModule_add_window(my_script_state);
    TeaModule_add_entity(my_script_state);
    TeaModule_add_pomegranate(my_script_state);
    
    //Get functions
    tea_push_null(my_script_state);
    tea_set_global(my_script_state,"editor_draw");
    
    tea_push_null(my_script_state);
    tea_set_global(my_script_state,"editor_update");
    
    tea_push_null(my_script_state);
    tea_set_global(my_script_state,"draw");
    
    tea_push_null(my_script_state);
    tea_set_global(my_script_state,"update");
    
    tea_push_null(my_script_state);
    tea_set_global(my_script_state,"ui");
    
    tea_push_null(my_script_state);
    tea_set_global(my_script_state,"editor_ui");
    
    tea_push_number(my_script_state,this->ID);
    tea_set_global(my_script_state,"self");
}

ScriptableEntity::~ScriptableEntity()
{
    tea_close(my_script_state);
}

void ScriptableEntity::update(float delta)
{
    if(this->opened)
    {
        tea_get_global(my_script_state,"update");
        if(tea_is_function(my_script_state,-1))
            tea_call(my_script_state,0);
    }
}

void ScriptableEntity::draw(float delta)
{
    if(this->opened)
    {
        tea_get_global(my_script_state,"draw");
        if(tea_is_function(my_script_state,-1))
            tea_call(my_script_state,0);
    }
}


void ScriptableEntity::editor_update(float delta)
{
    script_string = input_file_string("res/scripts/"+this->script_src);
    if(script_string!=lst_src_script)
    {
        tea_interpret(my_script_state,"",script_string.c_str());
        lst_src_script = script_string;
        this->opened = true;
    }
    if(this->opened)
    {
        tea_get_global(my_script_state,"editor_update");
        if(tea_is_function(my_script_state,-1))
            tea_call(my_script_state,0);
    }
}

void ScriptableEntity::editor_draw(float delta)
{
    if(this->opened)
    {
        tea_get_global(my_script_state,"editor_draw");
        if(tea_is_function(my_script_state,-1))
            tea_call(my_script_state,0);
    }
}

void ScriptableEntity::ui(float delta)
{
    if(this->opened)
    {
        tea_get_global(my_script_state,"ui");
        if(tea_is_function(my_script_state,-1))
            tea_call(my_script_state,0);
    }
}

void ScriptableEntity::editor_ui(float delta)
{
    if(this->opened)
    {
        tea_get_global(my_script_state,"editor_ui");
        if(tea_is_function(my_script_state,-1))
            tea_call(my_script_state,0);
    }
}