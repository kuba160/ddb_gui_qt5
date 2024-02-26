#version 440

# define M_PI           3.14159265358979323846  /* pi */

layout(location = 0) in vec2 coords;
layout(location = 0) out vec4 fragColor;

layout(binding = 0) uniform colors {
    vec4 fg;
    vec4 bg;
};

layout(binding = 1) readonly restrict buffer buf {
    int channels;
    int point_count;
    int width;
    int height;
    vec2 scope_point[];
};

#define POLAR 1

void main() {
    float value = 0.0;

#if POLAR
    vec2 shifted = vec2(coords.x -.5, coords.y -.5);
    float r = length(shifted);
    float theta = atan(shifted.y, shifted.x);
    int x_pos = int((degrees(theta)+180.0)/360.0  * point_count);
#else
    int x_pos = max(0, min(point_count, int(coords.x * point_count)));
#endif

    for (int i = 0; i < channels; i++) {
#if POLAR
        vec2 y_range =(scope_point[i*point_count+x_pos] - (channels-1-i)) / (4.0/float(channels));
        float ymin = y_range.x;
        float ymax = y_range.y;

        // polar approx
        float deg_approx = (channels-i) * .2;
        int x_pos_min = max(0, int((degrees(theta)+180.0-deg_approx)/360.0  * point_count));
        int x_pos_max = min(point_count-1, int((degrees(theta)+180.0+deg_approx)/360.0  * point_count));
        for (int j = x_pos_min; j <= x_pos_max; j++) {
            vec2 y_range =(scope_point[i*point_count+j] - (channels-1-i)) / (4.0/float(channels));
            ymin = min(ymin, y_range.x);
            ymax = max(ymax, y_range.y);
        }
        if (abs(ymax-ymin) < 1.0/width) {
            ymin -= .7/width;
            ymax += .7/width;
        }

        value += float(r < ((float(i)/float(channels) + ymax) / float(channels)) &&
                       r > ((float(i)/float(channels) + ymin) / float(channels)));
#else
        float y = coords[1]*channels - (channels - i - 1);
        vec2 y_range = (scope_point[i*point_count+x_pos] + i) - 1 ;
        float ymin = y_range.x;
        float ymax = y_range.y;
        float diff = (max(abs(ymin), abs(ymax)) - min(abs(ymin), abs(ymax)));
        if (1.0/float(height) >= float(abs(diff))) {
            ymin = (ymin-diff/2.0) -  1.25/float(height)*2 ;
            ymax = (ymin+diff/2.0) + 1.25/float(height) *2;
        }

        value = value + smoothstep(ymin, ymin, y) * smoothstep(-ymax, -ymax, -y);
#endif
    }
    fragColor = fg * value + bg * (1 - value);
}
