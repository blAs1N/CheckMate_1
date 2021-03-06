#include "MapManager.h"

void MapRead() {
	char mapName[20];

	sprintf(mapName, "Stage%d.txt", nowStage);

	FILE* mapFile = fopen(mapName, "r");
	if (mapFile == NULL) return;

	mapSize = GetSize(mapFile);

	if (cellMap != NULL) {
		SAFE_FREE(cellMap);
		SAFE_FREE(bombMap);
	}

	cellMap = (char**)malloc(mapSize.cy * sizeof(char*));
	bombMap = (char**)malloc(mapSize.cy * sizeof(char*));

#pragma omp parallel for
	for (int y = 0; y < mapSize.cy; y++) {
		// cellMap 1줄 초기화
		cellMap[y] = (char*)malloc(mapSize.cx + 1);
		memset(cellMap[y], 0, mapSize.cx + 1);
		fscanf(mapFile, "%s", cellMap[y]);

		// bombMap 1줄 초기화
		bombMap[y] = (char*)malloc(mapSize.cx + 1);
		memset(bombMap[y], 0, mapSize.cx + 1);
		
#pragma omp parallel for
		for (int x = 0; x < mapSize.cx; x++)
			bombMap[y][x] = '0';

#pragma omp parallel for
		for (int x = 0; x < mapSize.cx; x++) {
			// 플레이어가 해당 위치에 있는지 확인
			if (cellMap[y][x] == '2') {
				playerPos.x = x;
				playerPos.y = y;
			}

			// 적이 해당 위치에 있는지 확인
			if (cellMap[y][x] == '3') {
				POINT* enemyPos = (POINT*)malloc(sizeof(POINT));
				enemyPos->x = x;
				enemyPos->y = y;

				HANDLE enemyThread = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)AI, (LPVOID)enemyPos, 0, NULL);
				CloseHandle(enemyThread);
			}
		}
	}

	fclose(mapFile);
}

SIZE GetSize(FILE* mapFile) {
	SIZE mapSize;
	int size;

	// 파일의 끝으로가 전체 크기를 구한다.
	fseek(mapFile, 0, SEEK_END);
	size = ftell(mapFile);

	char tempMap[50];

	// 한줄 끝으로가 Y값을 구하고 전체와 나눠 X값을 구한다.
	rewind(mapFile);
	fgets(tempMap, 50, mapFile);
	mapSize.cx = ftell(mapFile) - 1;
	mapSize.cy = size / mapSize.cx;
	rewind(mapFile);

	return mapSize;
}

void MapDataFree() {
	for (int i = 0; i < mapSize.cy; i++) {
		cellMap[i] = NULL;
		free(cellMap[i]);
	}

	cellMap = NULL;
	free(cellMap);
}

void PlayerMoveInCell(POINT newPos) {
	cellMap[playerPos.y][playerPos.x] = '0';
	cellMap[newPos.y][newPos.x] = '2';

	playerPos = newPos;
}

void NextMap() {
	mapLoad = TRUE;

	if (nowStage >= 5) {
		bClear = TRUE;

		HDC hDC = GetDC(m_hWnd);
		RenderClear(hDC);
		ReleaseDC(m_hWnd, hDC);
	}

	else if (nowStage++ > 0) {
		HDC hDC = GetDC(m_hWnd);
		RenderLoad(hDC);
		ReleaseDC(m_hWnd, hDC);
		Sleep(2000);
	}
	
	MapRead();

	mapLoad = FALSE;
}