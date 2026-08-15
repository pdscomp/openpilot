#pragma once

// IRQs: CAN1_TX, CAN1_RX0, CAN1_SCE
//       CAN2_TX, CAN2_RX0, CAN2_SCE
//       CAN3_TX, CAN3_RX0, CAN3_SCE

#define CAN_ARRAY_SIZE 3
#define CAN_IRQS_ARRAY_SIZE 3
extern CAN_TypeDef *cans[CAN_ARRAY_SIZE];
extern uint8_t can_irq_number[CAN_IRQS_ARRAY_SIZE][CAN_IRQS_ARRAY_SIZE];
