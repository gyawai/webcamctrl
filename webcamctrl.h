
void pad_controller(int fd, const char *devname);
void key_controller(int fd);
void move_relative(int fd, int pan, int tilt);
void zoom_relative(int fd, int val);
void zoom_continuous(int fd, int val);
