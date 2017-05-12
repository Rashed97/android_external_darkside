/**
 * Copyright (C) 2015-2016 Guillaume Delugré
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * Copyright (C) 2017 Rashed Abdel-Tawab (rashed@linux.com)
 * Copyright (C) 2017 The LineageOS Project
 *
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "stache.h"
#include <cutils/log.h>
#include <private/android_filesystem_config.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define LOG_TAG "STACHE"

static int listen_fd = -1;
static struct sockaddr_un addr;

#define STACHE_SOCKET "/data/stache/stache_socket"

static
void usage(const char *program)
{
    fprintf(stderr, "Usage: %s [options] <command> [args...]\n", program);
    fprintf(stderr, "Helper tool to use ext4 encrypted directories.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Getting container information\n");
    fprintf(stderr, "  %s status <directory>\n", program);
    fprintf(stderr, "\n");
    fprintf(stderr, "Creating a new encrypted container:\n");
    fprintf(stderr, "  %s create <directory>\n", program);
    fprintf(stderr, "\n");
    fprintf(stderr, "Attaching to an existing encrypted container:\n");
    fprintf(stderr, "  %s attach <directory>\n", program);
    fprintf(stderr, "\n");
    fprintf(stderr, "Detaching from an encrypted container:\n");
    fprintf(stderr, "  %s detach <directory>\n", program);
    fprintf(stderr, "\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -p <LENGTH>:     Filename padding length (default is 4).\n");
    fprintf(stderr, "  -d <DESC>:       Key descriptor (up to 8 characters).\n");
    fprintf(stderr, "  -v:              Verbose output.\n");
}

static
bool is_valid_padding(unsigned padding)
{
    return ( padding == 4 || padding == 8 || padding == 16 || padding == 32 );
}

int open_socket()
{
    int rc = 0, stage = 0;
    int ret_fd;

	listen_fd = socket(PF_UNIX, SOCK_SEQPACKET, 0);
    if (listen_fd < 0) {
        stage = 1;
        goto error;
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, UNIX_PATH_MAX, STACHE_SOCKET);

    /* Delete existing socket file if necessary */
    unlink(addr.sun_path);

    rc = bind(listen_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un));
    if (rc != 0) {
        stage = 2;
        goto error;
    }

    chown(addr.sun_path, AID_ROOT, AID_SYSTEM);
    chmod(addr.sun_path, 0666);

    rc = listen(listen_fd, SOMAXCONN);
    if (rc != 0) {
        stage = 3;
        goto error;
    }

    while(1) {
        ret_fd = accept(listen_fd, (struct sockaddr*) NULL, NULL);
        printf("Recieved data on socket: %i", ret_fd);
        close(ret_fd);
        sleep(1);
    }


error:
    ALOGE("Unable to create stache control service (stage=%d, rc=%d)", stage, rc);
    if (listen_fd != -1) {
        close(listen_fd);
        listen_fd = -1;

        /* Delete socket file */
        unlink(addr.sun_path);
    }
    return 0;
}

void close_socket()
{
    if (listen_fd != -1) {
        close(listen_fd);
        listen_fd = -1;
        unlink(STACHE_SOCKET);
    }
}

int main(int argc, char **argv)
{
    open_socket();
#if 0
    close_socket();
#endif
}

int crypt(int argc, char *argv[])
{
    const char *program = basename(argv[0]);
    int c, opt_index;
    size_t desc_len;
    struct ext4_crypt_options opts = {
        .verbose = false,
        .contents_cipher = "aes-256-xts",
        .filename_cipher = "aes-256-cts",
        .filename_padding = 4,
        .key_descriptor = { 0 },
        .requires_descriptor = true,
    };

    while ( true ) {
        static struct option long_options[] = {
            { "help",         no_argument,        0, 'h' },
            { "verbose",      no_argument,        0, 'v' },
            { "name-padding", required_argument,  0, 'p' },
            { "key-desc",     required_argument,  0, 'd' },
            { 0, 0, 0, 0 },
        };

        c = getopt_long(argc, argv, "hvp:d:", long_options, &opt_index);
        if ( c == -1 )
            break;

        switch ( c ) {
            case 'h':
                usage(program);
                return EXIT_SUCCESS;

            case 'v':
                opts.verbose = true;
                break;

            case 'p':
                opts.filename_padding = atoi(optarg);
                if ( !is_valid_padding(opts.filename_padding) ) {
                    fprintf(stderr, "Invalid filename padding length: must be 4, 8, 16 or 32\n");
                    return EXIT_FAILURE;
                }
                break;

            case 'd':
                desc_len = strlen(optarg);
                if ( desc_len == 0 || desc_len > sizeof(opts.key_descriptor) ) {
                    fprintf(stderr, "Invalid descriptor %s: must be between 1 and 8 characters.", optarg);
                    return EXIT_FAILURE;
                }

                memcpy(opts.key_descriptor, optarg, desc_len);
                opts.requires_descriptor = false;
                break;

            default:
                usage(program);
                return EXIT_FAILURE;
        }
    }

    if ( optind + 1 >= argc ) {
        usage(program);
        return EXIT_FAILURE;
    }

    int status = 0;
    const char *command = argv[optind];
    const char *dir_path = argv[optind + 1];

    if ( strcmp(command, "help") == 0 ) {
        usage(program);
    }
    else if ( strcmp(command, "status") == 0 ) {
        status = container_status(dir_path);
    }
    else if ( strcmp(command, "create") == 0 ) {
        status = container_create(dir_path, opts);
    }
    else if ( strcmp(command, "attach") == 0 ) {
        status = container_attach(dir_path, opts);
    }
    else if ( strcmp(command, "detach") == 0 ) {
        status = container_detach(dir_path, opts);
    }
    else {
        fprintf(stderr, "Error: unrecognized command %s\n", command);
        usage(program);
        status = -1;
    }

    return (status == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
