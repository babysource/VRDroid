// This proprietary software may be used only as
// authorised by a licensing agreement from ARM Limited
// (C) COPYRIGHT 2015 ARM Limited
// ALL RIGHTS RESERVED
// The entire notice above must be reproduced on all authorised
// copies and copies may only be made to the extent permitted
// by a licensing agreement from ARM Limited.

char *read_file(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        LOGE("Failed to open file %s\n", filename);
        exit (1);
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *data = (char *)calloc(length + 1, sizeof(char));
    if (!data)
    {
        LOGE("Failed to allocate memory for file data %s\n", filename);
        exit(1);
    }
    size_t read = fread(data, sizeof(char), length, file);
    if (read != length)
    {
        LOGE("Failed to read whole file %s\n", filename);
        exit(1);
    }
    data[length] = '\0';
    fclose(file);
    return data;
}

GLuint compile_shader(const char *source, GLenum type)
{
    GLuint result = glCreateShader(type);
    glShaderSource(result, 1, (const GLchar**)&source, NULL);
    glCompileShader(result);
    GLint status;
    glGetShaderiv(result, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        GLint length;
        glGetShaderiv(result, GL_INFO_LOG_LENGTH, &length);
        GLchar *info = new GLchar[length];
        glGetShaderInfoLog(result, length, NULL, info);
        LOGE("[COMPILE] %s\n", info);
        delete[] info;
        exit(1);
    }
    return result;
}

GLuint link_program(GLuint *shaders, int count)
{
    GLuint program = glCreateProgram();
    for (int i = 0; i < count; ++i)
        glAttachShader(program, shaders[i]);

    glLinkProgram(program);

    for (int i = 0; i < count; ++i)
        glDetachShader(program, shaders[i]);

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        GLint length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        GLchar *info = new GLchar[length];
        glGetProgramInfoLog(program, length, NULL, info);
        LOGE("[LINK] %s\n", info);
        delete[] info;
        exit(1);
    }
    return program;
}

void load_cube_shader(App *app)
{
    char *vs_src = read_file(SHADER_PATH("cube.vs"));
    char *fs_src = read_file(SHADER_PATH("cube.fs"));

    GLuint shaders[2];
    shaders[0] = compile_shader(vs_src, GL_VERTEX_SHADER);
    shaders[1] = compile_shader(fs_src, GL_FRAGMENT_SHADER);
    app->program_cube = link_program(shaders, 2);

    free(vs_src);
    free(fs_src);
}

void load_distort_shader(App *app)
{
    char *vs_src = read_file(SHADER_PATH("distort.vs"));
    char *fs_src = read_file(SHADER_PATH("distort.fs"));

    GLuint shaders[2];
    shaders[0] = compile_shader(vs_src, GL_VERTEX_SHADER);
    shaders[1] = compile_shader(fs_src, GL_FRAGMENT_SHADER);
    app->program_distort = link_program(shaders, 2);

    free(vs_src);
    free(fs_src);
}

void load_assets(App *app)
{
    load_distort_shader(app);
    load_cube_shader(app);
}
