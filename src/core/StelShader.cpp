#include <QDebug>
#include <QString>

#include <QDir>
#include <QFile>

#include <GLee.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "StelShader.hpp"
#include "StelFileMgr.hpp"

static unsigned int loadShader(const char* path, unsigned int type);
static unsigned int createShader(const char *source, unsigned int type, const char *fileName = 0);
static unsigned int createProgram(unsigned int vertexShader, unsigned int pixelShader);

StelShader::StelShader()
{
        vertexShader = pixelShader = program = 0;
}

StelShader::~StelShader()
{
        if (program)
        {
                if (vertexShader)
                {
                        glDeleteShader(vertexShader);
                }
                if (pixelShader)
                {
                        glDeleteShader(pixelShader);
                }
                glDeleteProgram(program);
        }
}

int StelShader::uniformLocation(const char *name) const
{
        if (!(use()))
        {
                return -1;
        }
        return glGetUniformLocation(program, name);
}

int StelShader::attributeLocation(const char *name) const
{
        if (!use())
        {
                return -1;
        }
        return glGetAttribLocation(program, name);
}

void StelShader::setUniform(int location, int value)
{
        if (!use() || location == -1)
        {
                return;
        }
        glUniform1i(location, value);
}

void StelShader::setUniform(int location, int x, int y)
{
        if(!use() || location == -1)
        {
                return;
        }
        glUniform2i(location, x, y);
}

void StelShader::setUniform(int location, int x, int y, int z)
{
        if(!use() || location == -1)
        {
                return;
        }
        glUniform3i(location, x, y, z);
}

void StelShader::setUniform(int location, int x, int y, int z, int w)
{
        if(!use() || location == -1)
        {
                return;
        }
        glUniform4i(location, x, y, z, w);
}

void StelShader::setUniform(int location, float value)
{
        if (!use() || location == -1)
        {
                return;
        }
        glUniform1f(location, value);
}

void StelShader::setUniform(int location, float x, float y)
{
       if(!use() || location == -1)
       {
               return;
       }
       glUniform2f(location, x, y);
}

void StelShader::setUniform(int location, float x, float y, float z)
{
        if (!use() || location == -1)
        {
                return;
        }
        glUniform3f(location, x, y, z);
}

void StelShader::setUniform(int location, float x, float y, float z, float w)
{
        if(!use() || location == -1)
        {
                return;
        }
        glUniform4f(location, x, y, z, w);
}

bool StelShader::load(const char *vertexFile, const char *pixelFile)
{
        if (!(vertexShader = loadShader(vertexFile, GL_VERTEX_SHADER)))
        {
                return false;
        }
        if (!(pixelShader = loadShader(pixelFile, GL_FRAGMENT_SHADER)))
        {
                return false;
        }
        if (!(program = createProgram(vertexShader, pixelShader)))
        {
                return false;
        }
        return true;
}

bool StelShader::use() const
{
        if(program)
        {
                glUseProgram(program);
                return true;
        }
        return false;
}

bool useShader(const StelShader *shader)
{
        if(shader)
        {
                return shader->use();
        }
        glUseProgram(0);
        return true;
}

static unsigned int loadShader(const char* path, unsigned int type)
{
        FILE* fp;
        char* content;
        int size;
        unsigned int shader;

        QString shaderFile = StelFileMgr::findFile(path, StelFileMgr::File);
        if (!QFileInfo(shaderFile).exists())
        {
//               qWarning() << "Could not find file: " << shaderFile << "\n";
               fprintf(stderr, "Could not find file: %s\n", path);
               return 0;
        }
        qDebug() << "Loading shader: " << shaderFile << "...\n";

        if (!(fp = fopen(path, "r")))
        {
//                        qWarning() << "Could not open file: " << fileName << "...\n";
                fprintf(stderr, "Could not open shader %s: %s\n", path, strerror(errno));
                return 0;
        }

        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        rewind(fp);

        if (!(content = (char*) malloc(size + 1)))
        {
                perror("failed to allocate memory");
                return 0;
        }

        fread(content, 1, size, fp);
        content[size] = 0;
        fclose(fp);

        shader = createShader(content, type, path);
        free(content);
        return shader;
}

static unsigned int createShader(const char *source, unsigned int type, const char *fileName) {
        unsigned int shader;
        int status, logLength;
        char infoLog[512];

        if (!fileName)
        {
                fileName = "unknown";
        }

        if (!source) {
                return 0;
        }

        shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, 0);

        glCompileShader(shader);
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        glGetShaderInfoLog(shader, sizeof infoLog, &logLength, infoLog);

        if (status == GL_FALSE)
        {
//                qWarning << "Error while compiling " << fileName << ":\n" << infoLog;
                fprintf(stderr, "Error while compiling %s:\n%s\n", fileName, infoLog);
                glDeleteShader(shader);
                return 0;
        }
        else if (logLength)
        {
//                qWarning << *fileName << "\n" << infoLog << "\n";
                fprintf(stderr, "%s:\n%s\n", fileName, infoLog);
        }
        return shader;
}

static unsigned int createProgram(unsigned int vertexShader, unsigned int pixelShader)
{
        unsigned int program;
        int status, logLength;
        char infoLog[512];

        program = glCreateProgram();

        glAttachShader(program, vertexShader);
        glAttachShader(program, pixelShader);

        glLinkProgram(program);

        glGetProgramiv(program, GL_LINK_STATUS, &status);
        glGetProgramInfoLog(program, sizeof infoLog, &logLength, infoLog);

        if (status == GL_FALSE)
        {
//                qWarning << "Error while linking shader program:\n" << infoLog << "\n";
                fprintf(stderr, "Error while linking shader program:\n%s\n", infoLog);
                glDeleteProgram(program);
                return 0;
        }
        else if (logLength)
        {
//                qWarning << "\n" << infoLog << "\n";
                fprintf(stderr, "%s\n", infoLog);
        }

        return program;
}
