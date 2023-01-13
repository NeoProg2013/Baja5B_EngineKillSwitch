//  ***************************************************************************
/// @file    main.c
/// @author  NeoProg
//  ***************************************************************************
#include "project_base.h"
#include "systimer.h"

static void system_init(void);



/// ***************************************************************************
/// @brief  Program entry point
/// @param  none
/// @return none
/// ***************************************************************************
void fail_safe_loop() {
    while (true) {
        gpio_set(GPIOA, 10);
        
        static uint64_t start_blink_time = 0;
        if (get_time_ms() - start_blink_time > 50) {
            gpio_toggle(GPIOA, 3);
            start_blink_time = get_time_ms();
        }
    }
}

static uint16_t width = 0;
int main() {
    system_init();
    systimer_init();
    
    gpio_set_mode(GPIOA, 9, GPIO_MODE_INPUT);
    gpio_set_pull(GPIOA, 9, GPIO_PULL_UP);
    
    gpio_set(GPIOA, 10);
    gpio_set_mode(GPIOA, 10, GPIO_MODE_OUTPUT);
    gpio_set_output_type(GPIOA, 10, GPIO_TYPE_OPEN_DRAIN);
    
    gpio_set(GPIOA, 3);
    gpio_set_mode(GPIOA, 3, GPIO_MODE_OUTPUT);
    gpio_set_output_type(GPIOA, 3, GPIO_TYPE_PUSH_PULL);
    
    RCC->APB1RSTR |= RCC_APB1RSTR_TIM14RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM14RST;
    TIM14->CR1 = TIM_CR1_UDIS;
    TIM14->PSC = APB1_CLOCK_FREQUENCY / 1000000;
    TIM14->CNT = 0;
    
    
    delay_ms(1000);
    
    uint64_t start_timeout = get_time_ms();
    while (true) {
        
        start_timeout = get_time_ms();
        while (gpio_read_input(GPIOA, 9) == 0) {
            if (get_time_ms() - start_timeout > 200) {
                fail_safe_loop();
            }
        }

        TIM14->CNT = 0;
        TIM14->CR1 = TIM_CR1_CEN;
        TIM14->EGR = TIM_EGR_UG;
        start_timeout = get_time_ms();
        while (gpio_read_input(GPIOA, 9) == 1) {
            if (get_time_ms() - start_timeout > 200) {
                fail_safe_loop();
            }
        }
        TIM14->CR1 &= ~TIM_CR1_CEN;
        
        width = TIM14->CNT;
        if (width > 1500) {
            gpio_reset(GPIOA, 10);
        } else {
            gpio_set(GPIOA, 10);
        }
        
        
        static uint64_t start_blink_time = 0;
        if (get_time_ms() - start_blink_time > 1000) {
            gpio_toggle(GPIOA, 3);
            start_blink_time = get_time_ms();
        }
    }
}

void HardFault_Handler(void) {
    NVIC_SystemReset();
}

/// ***************************************************************************
/// @brief  System initialization
/// @param  none
/// @return none
/// ***************************************************************************
static void system_init(void) {
    // Enable Prefetch Buffer
    FLASH->ACR = FLASH_ACR_PRFTBE;
    
    /*// Enable HSE
    RCC->CR |= RCC_CR_HSEON;
    while ((RCC->CFGR & RCC_CR_HSERDY) == 0);
    
    // Configure PLL (clock source HSI/2 = 4MHz)
    RCC->CFGR |= RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL2;
    RCC->CR |= RCC_CR_PLLON;
    while ((RCC->CR & RCC_CR_PLLRDY) == 0);
    
    // Set FLASH latency
    FLASH->ACR |= FLASH_ACR_LATENCY;
    
    // Switch system clock to PLL
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS_PLL) == 0);*/

    // Enable GPIO clocks
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    
    // Enable TIM14 clocks
    RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
}
