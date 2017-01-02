#ifndef SOCKET_H
#define SOCKET_H

class Socket
{
public:
	explicit Socket(int fd):mfd(fd);
	~Socket();

private:
	const int mfd;
};

#endif

