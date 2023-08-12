#include "Memory/Memory.h"
#include "Offsets/Offsets.h"
#include "Utils/Vector3.h"

#include <iostream>

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

            const int32_t playerTeam = memory.Read<std::int32_t>(player + hazedumper::netvars::m_iTeamNum);
            const Vector3 playerEyePosition = memory.Read<Vector3>(player + hazedumper::netvars::m_vecOrigin) +
                memory.Read<Vector3>(player + hazedumper::netvars::m_vecViewOffset);
            const Vector3 playerViewAngles = memory.Read<Vector3>
                (clientState + hazedumper::signatures::dwClientState_ViewAngles);
            const Vector3 playerViewVector = playerViewAngles.anglesToDirection();

            

            // Dirección base de la lista de entidades
            const unsigned int entityList = client + hazedumper::signatures::dwEntityList;
            float minDistance = INFINITY;
            Vector3 anglesToAimAt{ 0, 0, 0 };
            for (int i = 1; i <= 32; i++) {
                // Dirección base de la entidad
                const uintptr_t enemy = memory.Read<std::uintptr_t>
                    (client + hazedumper::signatures::dwEntityList + i * 0x10);

                if (memory.Read<std::int32_t>(enemy + hazedumper::netvars::m_iTeamNum) == playerTeam)
                    continue;
                if (memory.Read<bool>(enemy + hazedumper::signatures::m_bDormant))
                    continue;
                if (memory.Read<std::int32_t>(enemy + hazedumper::netvars::m_lifeState))
                    continue;

                const uintptr_t boneMatrix = memory.Read<std::uintptr_t>
                    (enemy + hazedumper::netvars::m_dwBoneMatrix);
                const Vector3 enemyEyePosition = Vector3{
                        memory.Read<float>(boneMatrix + 0x30 * 8 + 0x0C),
                        memory.Read<float>(boneMatrix + 0x30 * 8 + 0x1C),
                        memory.Read<float>(boneMatrix + 0x30 * 8 + 0x2C)
                };

                if (!enemyEyePosition.isZero()) {
                    const Vector3 playerToEnemyVector = normalize(enemyEyePosition - playerEyePosition);
                    const float enemyDistance = distance(playerViewVector, playerToEnemyVector);
                    if (enemyDistance < minDistance) {
                        minDistance = enemyDistance;
                        anglesToAimAt = Vector3{
                            asin(playerToEnemyVector.z / magnitude(playerToEnemyVector)),
                            atan2(playerToEnemyVector.y, playerToEnemyVector.x),
                            0
                        }.radiansToDegrees();
                    }
                }
            }

            if (GetAsyncKeyState(VK_RBUTTON) && !anglesToAimAt.isZero()) {
                memory.Write<Vector3>(clientState + hazedumper::signatures::dwClientState_ViewAngles, anglesToAimAt);
            }
            
            if (GetAsyncKeyState(VK_LBUTTON)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }
    else {
        std::cout << "No se encontró el proceso de nombre csgo.exe" << std::endl;
    }
}