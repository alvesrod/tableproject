#ifndef IMPORTANTCONSTANTS_H
#define IMPORTANTCONSTANTS_H

/*
 ---------------
   Add here any major constant in the application that you
   feel like it will be changed somewhere in the future.
 ---------------

   The Engine doesn't know about these constants.
*/



/** ================ APP VERSION ================ */

/*
 * The version is important to not allow clients with
 * different versions to join the same room. This version
 * must match the version stored in the web server.
 * Otherwise, users will receive a popup suggesting them to
 * update to the latest version. Updates are done automatically.
 */

#define APP_VERSION "0.6b"

/*
 * The Engine version below must match the version in the Engine library.
 * Otherwise, the application won't start. This is to prevent users
 * using outdated versions of the Engine.
 */

#define ENG_VERSION "0.3b"

/** ============================================= */



/** ================ APP INFO =================== */

/*
 * Used to save information in the register:
 * They are also used to avoid users with different
 * application names from trading network messages.
 */

#define APP_NAME "Project Live Table"

#define APP_COMPANY "UBC"

/*
 * Key necessary to fetch info from the database and trade
 * network messages. The network messages between users will be blocked if
 * they don't have the same secret key.
 * Ps: In order to publish rooms, you must also set the APP_SECRET
 * in the server side (through the config link in the web server).
 */
#define APP_SECRET "INSERT_HERE_APP_SECRET"

/** @warning IF YOU SHARE THIS CODE TO THE PUBLIC, YOU MUST CHANGE THE APP_SECRET! **/

/** ============================================= */



/** ================= WEB SERVERS =============== */

#define SERVERS_CONTACT_ADDR "http://www.projectlivetable.com" //URL where connect.php is stored

#define TOTAL_IPS_TO_FETCH 200 //The maximum number of ips that will be fetched

/** ============================================= */


/** ============= FIND ROOM OPTIONS ============= */

 /*
  * If the list doesn't arrive in that time, display a message saying that there's no connection:
  */
#define TIME_WAITING_LIST_SECONDS 5

/*
 * If X rooms in a list doesn't work in a row, stop searching and assume the rest wont work:
 */
#define MAX_NUMBER_OF_CONSECUTIVE_FAILS 15

/*
 * Total of rooms that will be pinged simultaneously:
 */
#define MAX_NUMBER_OF_RUNNING_TASKS 10
/** ============================================= */



/** =============== LOADING ROOM ================ */

/*
 * The time limit that the loading screen can stay waiting for a host message
 * before getting an error message in the loading screen:
 */
#define MAX_LOADING_WAITING_TIME_SECONDS 20

/*
 * The time limit that the loading screen can spend loading
 * a room before getting an error message in the loading screen:
 */
#define MAX_LOADING_ROOM_SECONDS 15


/** ============================================= */


/** =================== TABLE =================== */

/*
 * The type of image files to be saved:
 */
#define IMAGE_FILE_TYPE "png"

/*
 * The type of table files:
 */
#define TABLE_FILE_TYPE "livetable"

/*
 * Autosaves a copy of the table file between those seconds.
 */
#define AUTO_SAVE_TIME_SECONDS 60;

/** ============================================= */


#endif // IMPORTANTCONSTANTS_H
