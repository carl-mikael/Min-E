#pragma once

void gl_debug_callback(unsigned int source, unsigned int type, unsigned int id,
                       unsigned int severity, int length, const char *message,
                       const void *userParam);

unsigned long glCheckError_(const char *file, int line);
