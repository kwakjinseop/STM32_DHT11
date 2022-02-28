#include <DHT.h>

#define lineDown() 		HAL_GPIO_WritePin(sensor->DHT_Port, sensor->DHT_Pin, GPIO_PIN_RESET)
#define lineUp()		HAL_GPIO_WritePin(sensor->DHT_Port, sensor->DHT_Pin, GPIO_PIN_SET)
#define getLine()		(HAL_GPIO_ReadPin(sensor->DHT_Port, sensor->DHT_Pin) == GPIO_PIN_SET)
#define Delay(d)		HAL_Delay(d)

static void goToOutput(DHT_sensor *sensor) {
	GPIO_InitTypeDef GPIO_InitStruct = {0};

  //기본적으로 높은 라인 유지
  lineUp();

  //출력 포트 설정
  GPIO_InitStruct.Pin = sensor->DHT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD; 	//
  if(sensor->pullUp == 1) {
	  GPIO_InitStruct.Pull = GPIO_PULLUP;						//전원 공급 장치
  } else {
	  GPIO_InitStruct.Pull = GPIO_NOPULL;						//
  }

  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; //고속포트 작동
  HAL_GPIO_Init(sensor->DHT_Port, &GPIO_InitStruct);
}

static void goToInput(DHT_sensor *sensor) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  //입력 포트 설정
  GPIO_InitStruct.Pin = sensor->DHT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  if(sensor->pullUp == 1) {
	  GPIO_InitStruct.Pull = GPIO_PULLUP;						//전원 공급 장치
  } else {
	  GPIO_InitStruct.Pull = GPIO_NOPULL;						//
  }
  HAL_GPIO_Init(sensor->DHT_Port, &GPIO_InitStruct);
}

DHT_data DHT_getData(DHT_sensor *sensor) {
	DHT_data data = {0.0f, 0.0f};

	#if DHT_POLLING_CONTROL == 1
	/* 센서폴링 주파수 제한 */
	//센서에 따라 폴링 간격 결정
	uint16_t pollingInterval;
	if (sensor->type == DHT11) {
		pollingInterval = DHT_POLLING_INTERVAL_DHT11;
	} else {
		pollingInterval = DHT_POLLING_INTERVAL_DHT22;
	}

	//주파수를 초과하면 마지막 성공값이 반환
	if (HAL_GetTick()-sensor->lastPollingTime < pollingInterval) {
		data.hum = sensor->lastHum;
		data.temp = sensor->lastTemp;
		return data;
	}
	sensor->lastPollingTime = HAL_GetTick();
	#endif

	/* 센서에 데이터 요청 */
	//핀을 출력으로 변환
	goToOutput(sensor);
	//데이터 라인 감소
	lineDown();
	Delay(15);
	//라인 리프팅
	lineUp();
	goToInput(sensor);

	/* 센서 응답 대기*/
	uint16_t timeout = 0;

	while(getLine()) {
		timeout++;
		if (timeout > DHT_TIMEOUT) return data;
	}
	timeout = 0;

	while(!getLine()) {
		timeout++;
		if (timeout > DHT_TIMEOUT) return data;
	}
	timeout = 0;

	while(getLine()) {
		timeout++;
		if (timeout > DHT_TIMEOUT) return data;
	}

	/* 센서 응답 읽기 */
	uint8_t rawData[5] = {0,0,0,0,0};
	for(uint8_t a = 0; a < 5; a++) {
		for(uint8_t b = 7; b != 255; b--) {
			uint32_t hT = 0, lT = 0;
			//
			while(!getLine()) lT++;
			//
			timeout = 0;
			while(getLine()) hT++;
			//
			if(hT > lT) rawData[a] |= (1<<b);
		}
	}
	/*데이터 무결성 검사 */
	if((uint8_t)(rawData[0] + rawData[1] + rawData[2] + rawData[3]) == rawData[4]) {
		// 체크섬이 일치하면 변환 및 반환 값
		if (sensor->type == DHT22) {
			data.hum = (float)(((uint16_t)rawData[0]<<8) | rawData[1])*0.1f;
			//음의 온도 검사
			if(!(rawData[2] & (1<<7))) {
				data.temp = (float)(((uint16_t)rawData[2]<<8) | rawData[3])*0.1f;
			}	else {
				rawData[2] &= ~(1<<7);
				data.temp = (float)(((uint16_t)rawData[2]<<8) | rawData[3])*-0.1f;
			}
		}
		if (sensor->type == DHT11) {
			data.hum = (float)rawData[0];
			data.temp = (float)rawData[2];;
		}
	}

	#if DHT_POLLING_CONTROL == 1
	sensor->lastHum = data.hum;
	sensor->lastTemp = data.temp;
	#endif

	return data;
}
