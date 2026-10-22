package com.vndroid.opengl;

import com.vndroid.util.AffineMatrix;

import android.opengl.GLES11Ext;
import android.opengl.GLES20;

import java.nio.FloatBuffer;

public final class AffineOpenGLFilter implements OpenGLFilter {

    private int program;
    private FloatBuffer vertexBuffer;
    private FloatBuffer texCoordsBuffer;
    private final float[] userMatrix;
    private final float saturation;

    private int vertexPosLoc;
    private int texCoordsInLoc;

    private int texLoc;
    private int texMatrixLoc;
    private int userMatrixLoc;
    private int saturationLoc;

    /**
     * Create a filter if a transform or a saturation change is requested, or return null if the frames may be used as is.
     */
    public static AffineOpenGLFilter createIfNeeded(AffineMatrix transform, float saturation) {
        if (transform == null && saturation == 1) {
            return null;
        }
        return new AffineOpenGLFilter(transform, saturation);
    }

    private AffineOpenGLFilter(AffineMatrix transform, float saturation) {
        // transform may be null if the filter only applies the saturation
        userMatrix = (transform != null ? transform : AffineMatrix.IDENTITY).to4x4();
        this.saturation = saturation;
    }

    @Override
    public void init() throws OpenGLException {
        // @formatter:off
        String vertexShaderCode = "#version 100\n"
                + "attribute vec4 vertex_pos;\n"
                + "attribute vec4 tex_coords_in;\n"
                + "varying vec2 tex_coords;\n"
                + "uniform mat4 tex_matrix;\n"
                + "uniform mat4 user_matrix;\n"
                + "void main() {\n"
                + "    gl_Position = vertex_pos;\n"
                + "    tex_coords = (tex_matrix * user_matrix * tex_coords_in).xy;\n"
                + "}";

        // @formatter:off
        String fragmentShaderCode = "#version 100\n"
                + "#extension GL_OES_EGL_image_external : require\n"
                + "precision highp float;\n"
                + "uniform samplerExternalOES tex;\n"
                + "uniform float saturation;\n"
                + "varying vec2 tex_coords;\n"
                + "void main() {\n"
                + "    if (tex_coords.x >= 0.0 && tex_coords.x <= 1.0\n"
                + "            && tex_coords.y >= 0.0 && tex_coords.y <= 1.0) {\n"
                + "        vec4 color = texture2D(tex, tex_coords);\n"
                + "        float luma = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));\n"
                + "        gl_FragColor = vec4(mix(vec3(luma), color.rgb, saturation), color.a);\n"
                + "    } else {\n"
                + "        gl_FragColor = vec4(0.0);\n"
                + "    }\n"
                + "}";

        program = GLUtils.createProgram(vertexShaderCode, fragmentShaderCode);
        if (program == 0) {
            throw new OpenGLException("Cannot create OpenGL program");
        }

        float[] vertices = {
                -1, -1, // Bottom-left
                1, -1, // Bottom-right
                -1, 1, // Top-left
                1, 1, // Top-right
        };

        float[] texCoords = {
                0, 0, // Bottom-left
                1, 0, // Bottom-right
                0, 1, // Top-left
                1, 1, // Top-right
        };

        // OpenGL will fill the 3rd and 4th coordinates of the vec4 automatically with 0.0 and 1.0 respectively
        vertexBuffer = GLUtils.createFloatBuffer(vertices);
        texCoordsBuffer = GLUtils.createFloatBuffer(texCoords);

        vertexPosLoc = GLES20.glGetAttribLocation(program, "vertex_pos");
        assert vertexPosLoc != -1;

        texCoordsInLoc = GLES20.glGetAttribLocation(program, "tex_coords_in");
        assert texCoordsInLoc != -1;

        texLoc = GLES20.glGetUniformLocation(program, "tex");
        assert texLoc != -1;

        texMatrixLoc = GLES20.glGetUniformLocation(program, "tex_matrix");
        assert texMatrixLoc != -1;

        userMatrixLoc = GLES20.glGetUniformLocation(program, "user_matrix");
        assert userMatrixLoc != -1;

        saturationLoc = GLES20.glGetUniformLocation(program, "saturation");
        assert saturationLoc != -1;
    }

    @Override
    public void draw(int textureId, float[] texMatrix) {
        GLES20.glUseProgram(program);
        GLUtils.checkGlError();

        GLES20.glEnableVertexAttribArray(vertexPosLoc);
        GLUtils.checkGlError();
        GLES20.glEnableVertexAttribArray(texCoordsInLoc);
        GLUtils.checkGlError();

        GLES20.glVertexAttribPointer(vertexPosLoc, 2, GLES20.GL_FLOAT, false, 0, vertexBuffer);
        GLUtils.checkGlError();
        GLES20.glVertexAttribPointer(texCoordsInLoc, 2, GLES20.GL_FLOAT, false, 0, texCoordsBuffer);
        GLUtils.checkGlError();

        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLUtils.checkGlError();
        GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, textureId);
        GLUtils.checkGlError();
        GLES20.glUniform1i(texLoc, 0);
        GLUtils.checkGlError();

        GLES20.glUniformMatrix4fv(texMatrixLoc, 1, false, texMatrix, 0);
        GLUtils.checkGlError();

        GLES20.glUniformMatrix4fv(userMatrixLoc, 1, false, userMatrix, 0);
        GLUtils.checkGlError();

        GLES20.glUniform1f(saturationLoc, saturation);
        GLUtils.checkGlError();

        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        GLUtils.checkGlError();
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
        GLUtils.checkGlError();
    }

    @Override
    public void release() {
        GLES20.glDeleteProgram(program);
        GLUtils.checkGlError();
    }
}
