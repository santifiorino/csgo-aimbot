#include "Memory/Memory.h"
#include "Offsets/Offsets.h"
#include "Utils/Vector3.h"

#include <iostream>

#include <chrono>
#include <thread>

int main() {
    std::cout << "Aimbot" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
    std::cout << "Buscando y abriendo proceso de nombre csgo.exe" << std::endl;
    // Abro el proceso csgo.exe para acceder a sus recursos
    const auto memory = Memory{ L"csgo.exe" };
    if (memory.isProcessOpened()) {
        std::cout << "Proceso encontrado!" << std::endl;
        /*
        Busco la base de la dirección de client.dll y de engine.dll
        Los archivos .dll son librerías compartidas que cada una le proveé
        diferentes funcionalidades al juego. El client.dll tiene código y
        datos sobre la funcionalidad del lado del cliente. Por ejemplo, lo
        necesario para renderizar los gráficos, manejar el input del usuario.
        El engine.dll tiene las funcionalidades principales del juego, como
        las simulaciones de físicas.
        */
        const uintptr_t client = memory.GetModuleAddress(L"client.dll");
        const uintptr_t engine = memory.GetModuleAddress(L"engine.dll");

        const uintptr_t clientState = memory.Read<std::uintptr_t>
            (engine + hazedumper::signatures::dwClientState);

        std::cout << "Aimbot activo" << std::endl;
        while (1) {

            // Dirección base del jugador
            const uintptr_t player = memory.Read<std::uintptr_t>
                (client + hazedumper::signatures::dwLocalPlayer);
            // Equipo del jugador
            const int32_t playerTeam = memory.Read<std::int32_t>(player + hazedumper::netvars::m_iTeamNum);
            // Coordenadas de los ojos del jugador
            const Vector3 playerEyePosition = memory.Read<Vector3>(player + hazedumper::netvars::m_vecOrigin) +
                memory.Read<Vector3>(player + hazedumper::netvars::m_vecViewOffset);
            // Angulos de los ojos del jugador
            const Vector3 playerViewAngles = memory.Read<Vector3>
                (clientState + hazedumper::signatures::dwClientState_ViewAngles);
            // Dirección donde apunta el jugador
            const Vector3 directionVector = playerViewAngles.toDirection();

            // Recorro las entidades para buscar al enemigo más cercano a la mira
            float minAngle = INFINITY;
            Vector3 anglesToAimAt{ 0, 0, 0 };
            // Dirección base de la lista de entidades
            const unsigned int entityList = client + hazedumper::signatures::dwEntityList;
            for (int i = 1; i <= 32; i++) {
                // Dirección base de la entidad
                const uintptr_t enemy = memory.Read<std::uintptr_t>
                    (client + hazedumper::signatures::dwEntityList + i * 0x10);
                // Si es de mi equipo lo ignoro
                if (memory.Read<std::int32_t>(enemy + hazedumper::netvars::m_iTeamNum) == playerTeam)
                    continue;
                /*
                Si está dormant lo ignoro (dormant=inactivo, pasa cuando están muy lejos o fuera de
                nuestra vista, en esos casos CSGO lo pone en estado dormant para ahorrarse cosas que
                mandar por la red y que ande todo mejor, cuando está inactivo no se recive
                actualizaciones de ese jugador ni se lo renderiza en nuestro cliente.)
                */
                if (memory.Read<bool>(enemy + hazedumper::signatures::m_bDormant))
                    continue;
                // Si está muerto lo ignoro
                if (memory.Read<std::int32_t>(enemy + hazedumper::netvars::m_lifeState))
                    continue;
                // Bone matrix del enemigo
                const uintptr_t boneMatrix = memory.Read<std::uintptr_t>
                    (enemy + hazedumper::netvars::m_dwBoneMatrix);
                // Posición de los ojos del enemigo
                const Vector3 enemyEyePosition = Vector3{
                        memory.Read<float>(boneMatrix + 0x30 * 8 + 0x0C),
                        memory.Read<float>(boneMatrix + 0x30 * 8 + 0x1C),
                        memory.Read<float>(boneMatrix + 0x30 * 8 + 0x2C)
                };
                if (!enemyEyePosition.isZero()) {
                    const Vector3 playerToEnemyVector = enemyEyePosition - playerEyePosition;
                    const float enemyXYAngle = atan2(playerToEnemyVector.y, playerToEnemyVector.x);
                    const float playerXYAngle = atan2(directionVector.y, directionVector.x);
                    if (abs(enemyXYAngle - playerXYAngle) < minAngle) {
                        minAngle = abs(enemyXYAngle - playerXYAngle);
                        anglesToAimAt = Vector3{
                            asin(playerToEnemyVector.z / magnitude(playerToEnemyVector)),
                            enemyXYAngle,
                            0
                        }.toDegrees();
                    }
                }
            }
            // Si toco click derecho apunta a la cabeza del enemigo
            if (GetAsyncKeyState(VK_RBUTTON) && !anglesToAimAt.isZero()) {
                memory.Write<Vector3>(clientState + hazedumper::signatures::dwClientState_ViewAngles, anglesToAimAt);
            }
            // Si toco click izquierdo (disparo) espero 100ms para que no cambie instantáneamente a otro enemigo apenas matas a uno
            if (GetAsyncKeyState(VK_LBUTTON)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    } else {
        std::cout << "No se encontró el proceso de nombre csgo.exe" << std::endl;
    }
}
