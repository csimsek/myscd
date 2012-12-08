#include "daemon.h"

pthread_mutex_t responder_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t checker_mutex = PTHREAD_MUTEX_INITIALIZER;

extern struct config g_config;
int g_run = FALSE, g_sd, g_run_checker = TRUE;
pthread_t g_checker_thread;

int check_daemon(void)
{
    FILE *pid_file;
    if ((pid_file = fopen(g_config.pid_file, "r")) == NULL)
        return FALSE;
    fclose(pid_file);
    if (lock_pid_file())
    {
        unlock_pid_file();
        return FALSE;
    }
    return TRUE;
}

void start_daemon(void)
{
    int pid;
    if (signal(SIGCHLD, signal_chld_handler) == SIG_ERR)
    {
        perror("signal");
        exit(-1);
    }
    
    pid = fork();
    if (pid)
        exit(0);
    else
    {
        int sd_client;
        int yes = 1;
        socklen_t addrlen;
        struct sockaddr_in sin;
        struct sockaddr_in pin;
        g_run = TRUE;

        start_checker();
        
        if (signal(SIGTERM, signal_term_handler) == SIG_ERR)
        {
            perror("signal");
            exit(-1);
        }

        if (signal(SIGHUP, signal_hup_handler) == SIG_ERR)
        {
            perror("signal");
            exit(-1);
        }

        check_create_pid();
        
        if ((g_sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            perror("socket");
            exit(-1);
        }

        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = INADDR_ANY;
        inet_aton(g_config.bind_address, &sin.sin_addr);
        sin.sin_port = htons(g_config.bind_port);

        setsockopt(g_sd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(g_sd, (struct sockaddr *) &sin, sizeof(sin)) == -1)
        {
            perror("bind");
            exit(-1);
        }

        if (listen(g_sd, g_config.max_clients) == -1)
        {
            perror("listen");
            exit(-1);
        }

        addrlen = sizeof(pin);

        while (g_run)
        {
            char buffer[1024];
            memset(buffer, 0, sizeof(buffer));
            if ((sd_client = accept(g_sd, (struct sockaddr *) &pin, &addrlen)) != -1)
            {
                pthread_t thread;
                struct client_info ci;
                pthread_attr_t tattr;
                ci.sd_client = sd_client;
                pthread_attr_init(&tattr);
                pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
                pthread_create(&thread, &tattr, responder, (void *) &ci);
                pthread_attr_destroy(&tattr);
            }
        }
        close(g_sd);
        unlock_pid_file();
        exit(0);
    }
}

void stop_daemon(void)
{
    int pid = read_pid_file();
    kill(pid, SIGTERM);
}

int lock_pid_file(void)
{
    struct flock fl;
    int fd;
    
    fl.l_type   = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start  = 0;
    fl.l_len    = 0;
    fl.l_pid    = getpid();

    fd = open(g_config.pid_file, O_WRONLY | O_CREAT, 0600);
    if (fcntl(fd, F_SETLK, &fl) == -1)
        return FALSE;
    fcntl(fd, F_SETLKW, &fl);
    return TRUE;
}

void unlock_pid_file(void)
{
    struct flock fl;
    int fd;
    
    fl.l_type   = F_UNLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start  = 0;
    fl.l_len    = 0;
    fl.l_pid    = getpid();

    fd = open(g_config.pid_file, O_WRONLY | O_CREAT, 0600);
    fcntl(fd, F_SETLK, &fl);
}


void check_create_pid(void)
{
    FILE *pid_file;
    char *pid_buffer;
    size_t result;
    
    if ((pid_file = fopen(g_config.pid_file, "wb")) == NULL)
    {
        fprintf(stderr, "unable to create pid file.\n");
        exit(-1);
    }
    
    pid_buffer = itoa(getpid(), 10);
    result = fwrite(pid_buffer, 1, strlen(pid_buffer), pid_file);
    
    if (strlen(pid_buffer) != result)
    {
        fprintf(stderr, "unable to write pid to file.\n");
        fclose(pid_file);
        exit(-1);
    }
    
    fclose(pid_file);
        
    if (!lock_pid_file())
    {
        fprintf(stderr, "unable to lock pid file\n");
        exit(-1);
    }
}

int read_pid_file(void)
{
    FILE *pid_file;
    char *pid_buffer;
    int size, retval;
    
    if ((pid_file = fopen(g_config.pid_file, "rb")) == NULL)
    {
        fprintf(stderr, "unable to open pid file.\n");
        exit(-1);
    }

    fseek(pid_file, 0, SEEK_END);
    size = ftell(pid_file);
    rewind(pid_file);
    pid_buffer = (char *) malloc(size);
    if (fread(pid_buffer, 1, size, pid_file) != size)
    {
        fclose(pid_file);
        fprintf(stderr, "unable to read pid from file.\n");
        exit(-1);
    }
    retval = atoi(pid_buffer);
    free(pid_buffer);
    return retval;
}

void signal_term_handler(int s)
{
    g_run = FALSE;
    close(g_sd);
    g_run_checker = FALSE;
    pthread_join(g_checker_thread, NULL);
}

void signal_hup_handler(int s)
{
    fprintf(stderr, "HUP signal\n");
}

void signal_chld_handler(int s)
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void start_checker(void)
{
    pthread_attr_t tattr;
    g_run_checker = TRUE;
    pthread_attr_init(&tattr);
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
    pthread_create(&g_checker_thread, &tattr, checker, NULL);
    pthread_attr_destroy(&tattr);
}

void stop_checker(void)
{
    g_run_checker = FALSE;
    pthread_join(g_checker_thread, NULL);
    sleep(g_config.host_check_interval);
}
