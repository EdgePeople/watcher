#include <unistd.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <sys/inotify.h>
#include <curl/curl.h>

using namespace std;

#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))
#define WATCH_FLAGS (IN_CREATE)
#define POST_TO "127.0.0.1:5000/api/data"

string read_file_content(string filename)
{
    ifstream ifs(filename.c_str(), ios::in | ios::binary | ios::ate);

    ifstream::pos_type fileSize = ifs.tellg();
    ifs.seekg(0, ios::beg);

    // cout << "size: " << fileSize << endl;

    vector<char> bytes(fileSize);
    ifs.read(bytes.data(), fileSize);
    ifs.close();
    return string(bytes.data(), fileSize);
}

void post_content_data(string content)
{
    CURL *curl;
    CURLcode res;
    // char *post_data = &content[0];
    // const char *post_data =  ("{\"content\" : \"" + content + "\"}").c_str();
    const char *post_data = content.c_str();
    curl = curl_easy_init();
    if (curl)
    {
        curl_slist *plist = curl_slist_append(NULL,
                                              "Content-Type:application/json;charset=UTF-8");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, plist);
        curl_easy_setopt(curl, CURLOPT_URL, POST_TO);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);

        /* if we don't provide POSTFIELDSIZE, libcurl will strlen() by
       itself */
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(post_data));

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

        /* always cleanup */
        curl_easy_cleanup(curl);
    }
}

int main()
{
    int fd;
    int wd;
    char buffer[EVENT_BUF_LEN];

    fd = inotify_init();

    if (fd < 0)
    {
        perror("inotify_init");
    }

    wd = inotify_add_watch(fd, "./", WATCH_FLAGS);

    while (true)
    {
        int length, i = 0;

        length = read(fd, buffer, EVENT_BUF_LEN);

        if (length < 0)
        {
            perror("read");
        }

        while (i < length)
        {
            struct inotify_event *event = (struct inotify_event *)&buffer[i];
            if (event->len)
            {
                if (event->mask & IN_CREATE)
                {
                    if (!(event->mask & IN_ISDIR))
                    {
                        // usleep(10000);
                        string filename = (char *)event->name;
                        string content = read_file_content(filename);
                        // cout << "file: " << filename << " content: \n"
                        //  << content << endl;
                        post_content_data(content);
                        remove(filename.c_str());
                    }
                }
            }
            i += EVENT_SIZE + event->len;
        }
    }

    return 0;
}