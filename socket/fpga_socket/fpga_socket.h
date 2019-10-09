#ifndef __FPGA_SOCKET_H__
#define __FPGA_SOCKET_H__

/*
 * Link Layer Format
 *
 * +---------+---------+-----------+-----------------+-------+
 * | dst mac | src mac | data type |     payload     | crc32 |
 * +---------+---------+-----------+-----------------+-------+
 * |    6    |    6    |     2     |                 |   4   |
 * +---------+---------+-----------+-----------------+-------+
 *
 * FPGA Payload Format
 * +-------+-----------+----------+--------------+
 * |  R/W  |  address  |  length  |     data     |
 * +-------+-----------+----------+--------------+
 * |   1   |     4     |    4     |              |
 * +-------+-----------+----------+--------------+
 */

/**
 * open RGMII socket
 *
 * @param if_name: interface name;
 * @return: RGMII socket fd
 */
int fpga_open(const char *if_name);

/**
 * close RGMII socket
 *
 * @param sockfd: RGMII socket fd
 */
void fpga_close(int sockfd);

/**
 * RGMII socket write
 *
 * @param sockfd: RGMII socket fd
 * @param buf: Link Layer formatted data
 * @param len: length in bytes of buf need to write
 * @param flags: gives 0
 * @return: bytes of successfully wrote
 */
ssize_t fpga_write(int sockfd, const uint8_t *buf, size_t len, int flags);

/**
 * RGMII socket read
 *
 * @param sockfd: RGMII socket fd
 * @param buf: data buffer to read
 * @param len: buffer size in bytes
 * @param flags: gives 0
 * @return: bytes of successfully read
 */
ssize_t fpga_read(int sockfd, uint8_t *buf, size_t len, int flags);

#endif /* __FPGA_SOCKET_H__ */
