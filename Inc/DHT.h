#ifndef DHT_H_
#define DHT_H_

#include "main.h"

/* 설정 */
#define DHT_TIMEOUT 				10000	//함수가 빈 값을 반환하는 반복횟수
#define DHT_POLLING_CONTROL			1		//센서 폴링 빈도 확인
#define DHT_POLLING_INTERVAL_DHT11	2000	//DHT11 폴링 간격(0.5Hz 데이터시) 1500을 공급할 수 있습니다. 작동됩니다.
#define DHT_POLLING_INTERVAL_DHT22	1000	//DHT22 폴링 간격(1Hz 데이터시)

/* 센서에서 반환하는 데이터 구조*/
typedef struct {
	float hum;
	float temp;
} DHT_data;

/* 사용되는 센서 유형 */
typedef enum {
	DHT11,
	DHT22
} DHT_type;

/* 센서 객체 구조 */
typedef struct {
	GPIO_TypeDef *DHT_Port;	//센서 포트 (GPIOA, GPIOB, etc)
	uint16_t DHT_Pin;		//센서 핀 번호 (GPIO_PIN_0, GPIO_PIN_1, etc)
	DHT_type type;			//센서 유형 (DHT11 или DHT22)
	uint8_t pullUp;			//전원 공급 장치 (0 - 없음, 1 - 있음)

	//센서 폴링 빈도 제어, 값을 채우지 말 것
	#if DHT_POLLING_CONTROL == 1
	uint32_t lastPollingTime;//마지막 센서 폴링 시간
	float lastTemp;			 //마지막 온도 값
	float lastHum;			 //마지막 습도 값
	#endif
} DHT_sensor;


/* 함수 실행 부분 */
DHT_data DHT_getData(DHT_sensor *sensor); //센서에서 데이터를 가져옴

#endif
