/*
 * cli.hpp
 *
 *  Created on: May 1, 2025
 *      Author: cavem
 */

#ifndef CLI_HPP_
#define CLI_HPP_

#ifdef __cplusplus
extern "C" {
#endif

int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t Len);
void USBTask(void* argument);

#ifdef __cplusplus
}
#endif

#endif /* CLI_HPP_ */
