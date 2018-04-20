#ifndef LOADER_H
#define LOADER_H


#define PAYLOAD_SIZE 60


namespace Loader
{
    bool getStartUploading(void);
    void setStartUploading(bool start);

    void upload(int vgaY);
}

#endif
