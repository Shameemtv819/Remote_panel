/**
 * @file scorpio_conf.h
 * @author Vetrivel
 * @brief 
 * @date 2025-03-13
 * @copyright Copyright (c) 2025
 */
/************************************************ Define to prevent recursive inclusion ********************************************/
#ifndef SCORPIO_CONF_H
#define SCORPIO_CONF_H
/******************************************************** Header includes **********************************************************/
/*********************************************************** Macros ****************************************************************/
#define FREE_RTOS                   (1)
#define PRODUCTION_BOARD            (1)
#define AWS_SERVER                  (1)
#define GOOGLE_SERVER               (0) // fire base
#define AZURE_SERVER                (0)

#define GSM_ENABLE                          (1)   // module enable/ disable
#define DEBUG_ENABLE                        (1)   // module enable/ disable
#define MCU_COMM_ENABLE                     (1)   // module enable/ disable  
#define HARDWARE_CRC_ENABLE                 (0)   // module enable/ disable

#define DEV_TEST                            (0) // for incrementing value - sending data to sms
#define DEV_DEBUG_PROTOCOL                  (0) // to download bin file and display through serial monitor
#define TEST_LTE_FW_DWNLD_DEBUG             (0) // sends test queue to firmware download
#define TEST_LTE_SMS                        (0) // sends test queue to sms
#define TEST_LTE_SIM_SWITCH                 (0) // sends test queue to switch
#define TEST_ETH_FW_DWNLD_DEBUG             (0)
#define TEST_ETH_PANEL_UPLOAD               (1U)

#if(PRODUCTION_BOARD)
//relay pin configuration
#define RL_1_PORT                          (GPIOD)
#define RL_2_PORT                          (GPIOD)
#define RL_1_PIN                           (GPIO_PIN_4)
#define RL_2_PIN                           (GPIO_PIN_7)
#define RELAY_1                            (0U)
#define RELAY_2                            (1U)

#define UART_LTE                            (huart1)
#define UART_LTE_INSTANCE                   (USART1)
#define UART_LTE_BAUDRATE                   (115200)  
#define UART_LTE_PORT                       (GPIOB)              
#define UART_LTE_PIN_TX                     (GPIO_PIN_7)
#define UART_LTE_PIN_RX                     (GPIO_PIN_6)
#define UART_LTE_PORT_CLOCK_EN()            __HAL_RCC_GPIOB_CLK_ENABLE()  
#define UART_LTE_PERI_CLOCK_EN()            __HAL_RCC_USART1_CLK_ENABLE()  
#define UART_LTE_PERI_CLOCK_DIS()           __HAL_RCC_USART1_CLK_DISABLE()  
#define UART_LTE_AF_MAP                     (GPIO_AF7_USART1)
#define UART_LTE_NVIC_IRQ                   (USART1_IRQn)
#define UART_LTE_IRQ_HANDLER                (USART1_IRQHandler)
#define UART_LTE_INTERRUPT_PRIO             (5)

#define UART_DEBUG                          (huart2)
#define UART_DEBUG_INSTANCE                 (USART2)
#define UART_DEBUG_BAUDRATE                 (115200)
#define UART_DEBUG_PORT                     (GPIOD)
#define UART_DEBUG_PIN_TX                   (GPIO_PIN_5)
#define UART_DEBUG_PIN_RX                   (GPIO_PIN_6)
#define UART_DEBUG_PORT_CLOCK_EN()          __HAL_RCC_GPIOD_CLK_ENABLE()
#define UART_DEBUG_PERI_CLOCK_EN()          __HAL_RCC_USART2_CLK_ENABLE()
#define UART_DEBUG_PERI_CLOCK_DIS()         __HAL_RCC_USART2_CLK_DISABLE()
#define UART_DEBUG_AF_MAP                   (GPIO_AF7_USART2)
#define UART_DEBUG_NVIC_IRQ                 (USART2_IRQn)
#define UART_DEBUG_IRQ_HANDLER              (USART2_IRQHandler)
#define UART_DEBUG_INTERRUPT_PRIO           (5)

#define UART_MCU_COMM                       (huart3)
#define UART_MCU_COMM_INSTANCE              (USART3)
#define UART_MCU_COMM_BAUDRATE              (115200)
#define UART_MCU_COMM_PORT                  (GPIOD)
#define UART_MCU_COMM_PIN_TX                (GPIO_PIN_8)
#define UART_MCU_COMM_PIN_RX                (GPIO_PIN_9)
#define UART_MCU_COMM_PORT_CLOCK_EN()       __HAL_RCC_GPIOD_CLK_ENABLE()
#define UART_MCU_COMM_PERI_CLOCK_EN()       __HAL_RCC_USART3_CLK_ENABLE()
#define UART_MCU_COMM_PERI_CLOCK_DIS()      __HAL_RCC_USART3_CLK_DISABLE()
#define UART_MCU_COMM_AF_MAP                (GPIO_AF7_USART3)
#define UART_MCU_COMM_NVIC_IRQ              (USART3_IRQn)
#define UART_MCU_COMM_IRQ_HANDLER           (USART3_IRQHandler)
#define UART_MCU_COMM_INTERRUPT_PRIO        (5)

#else // nucleo board

#define UART_LTE                            (huart2)
#define UART_LTE_INSTANCE                   (USART2)
#define UART_LTE_BAUDRATE                   (115200)  
#define UART_LTE_PORT                       (GPIOD)              
#define UART_LTE_PIN_TX                     (GPIO_PIN_5)
#define UART_LTE_PIN_RX                     (GPIO_PIN_6)
#define UART_LTE_PORT_CLOCK_EN()            __HAL_RCC_GPIOD_CLK_ENABLE()  
#define UART_LTE_PERI_CLOCK_EN()            __HAL_RCC_USART2_CLK_ENABLE()  
#define UART_LTE_PERI_CLOCK_DIS()           __HAL_RCC_USART2_CLK_DISABLE()  
#define UART_LTE_AF_MAP                     (GPIO_AF7_USART2)
#define UART_LTE_NVIC_IRQ                   (USART2_IRQn)
#define UART_LTE_IRQ_HANDLER                (USART2_IRQHandler)
#define UART_LTE_INTERRUPT_PRIO             (5)

#define UART_DEBUG                          (huart3)
#define UART_DEBUG_INSTANCE                 (USART3)
#define UART_DEBUG_BAUDRATE                 (115200)
#define UART_DEBUG_PORT                     (GPIOD)
#define UART_DEBUG_PIN_TX                   (GPIO_PIN_9)
#define UART_DEBUG_PIN_RX                   (GPIO_PIN_8)
#define UART_DEBUG_PORT_CLOCK_EN()          __HAL_RCC_GPIOD_CLK_ENABLE()
#define UART_DEBUG_PERI_CLOCK_EN()          __HAL_RCC_USART3_CLK_ENABLE()
#define UART_DEBUG_PERI_CLOCK_DIS()         __HAL_RCC_USART3_CLK_DISABLE()
#define UART_DEBUG_AF_MAP                   (GPIO_AF7_USART3)
#define UART_DEBUG_NVIC_IRQ                 (USART3_IRQn)
#define UART_DEBUG_IRQ_HANDLER              (USART3_IRQHandler)
#define UART_DEBUG_INTERRUPT_PRIO           (5)

#define UART_MCU_COMM                       (huart6)
#define UART_MCU_COMM_INSTANCE              (USART6)
#define UART_MCU_COMM_BAUDRATE              (115200)
#define UART_MCU_COMM_PORT                  (GPIOG)
#define UART_MCU_COMM_PIN_TX                (GPIO_PIN_14)
#define UART_MCU_COMM_PIN_RX                (GPIO_PIN_9)
#define UART_MCU_COMM_PORT_CLOCK_EN()       __HAL_RCC_GPIOG_CLK_ENABLE()
#define UART_MCU_COMM_PERI_CLOCK_EN()       __HAL_RCC_USART6_CLK_ENABLE()
#define UART_MCU_COMM_PERI_CLOCK_DIS()      __HAL_RCC_USART6_CLK_DISABLE()
#define UART_MCU_COMM_AF_MAP                (GPIO_AF8_USART6)
#define UART_MCU_COMM_NVIC_IRQ              (USART6_IRQn)
#define UART_MCU_COMM_IRQ_HANDLER           (USART6_IRQHandler)
#define UART_MCU_COMM_INTERRUPT_PRIO        (5)

#endif

/*********************************************************** typedef ***************************************************************/

/******************************************************** Extern variables *********************************************************/

#endif
