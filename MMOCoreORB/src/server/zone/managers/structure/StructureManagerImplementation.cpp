/*
 * StructureManagerImplementation.cpp
 *
 *  Created on: 23/07/2009
 *      Author: TheAnswer
 */

#include "StructureManager.h"

#include "server/zone/ZoneProcessServerImplementation.h"
#include "server/zone/Zone.h"
#include "server/db/ServerDatabase.h"
#include "server/zone/managers/object/ObjectManager.h"
#include "server/zone/objects/building/BuildingObject.h"

StructureManagerImplementation::StructureManagerImplementation(Zone* zone, ZoneProcessServerImplementation* processor) :
	ManagedObjectImplementation(), Logger("StructureManager") {
	StructureManagerImplementation::zone = zone;
	StructureManagerImplementation::server = processor;

	objectManager = processor->getZoneServer()->getObjectManager();

	setGlobalLogging(true);
	setLogging(true);
}

void StructureManagerImplementation::loadStaticBuildings() {
	int planetid = zone->getZoneID();

	//lock();

	StringBuffer query;
	query << "SELECT * FROM staticobjects WHERE zoneid = " << planetid << ";";

	try {
		ResultSet* result = ServerDatabase::instance()->executeQuery(query);

		while (result->next()) {
			uint64 oid = result->getUnsignedLong(1);

			uint64 parentId = result->getUnsignedLong(2);

			String file = result->getString(3);

			float oX = result->getFloat(4);
			float oY = result->getFloat(5);
			float oZ = result->getFloat(6);
			float oW = result->getFloat(7);

			float x = result->getFloat(8);
			float z = result->getFloat(9);
			float y = result->getFloat(10);

			float type = result->getFloat(11);
			bool client = result->getBoolean(12);

			//info("Loading Structures for zone: " + zone->getZoneID());

			if (file.indexOf("object/cell/") != -1) {
				BuildingObject* buio = (BuildingObject*) objectManager->get(parentId);

				if (buio == NULL)
					buio = loadStaticBuilding(parentId, planetid);

				StringBuffer msg;
				msg << "StructureManagerImplementation::loadStaticBuildings(), loading cell (" << oid << " for building: " << parentId << endl;
				info(msg.toString());

/*				CellObject* cell = new CellObject(oid, buio);
				cell->deploy();

				cell->setObjectCRC(file.hashCode());*/
				SceneObject* cell = objectManager->createObject(file.hashCode(), oid);
				cell->initializePosition(x, z, y);
				cell->setDirection(oX, oZ, oY, oW);

				//cell->setZoneProcessServer(server);
				//zone->registerObject(cell);

				buio->addCell((CellObject*)cell); // sets the cell number - assumes loading in order

				/*if (cellMap->put(oid, cell) != NULL) {
					error("Error CELL/BUILDING already exists\n");
					raise(SIGSEGV);
				}*/
			}
		}

		delete result;
	} catch (DatabaseException& e) {
		error(e.getMessage());
	} catch (...) {
		error("unreported exception caught in PlanetManagerImplementation::loadStaticBuildings()\n");
	}

	//unlock();
}

BuildingObject* StructureManagerImplementation::loadStaticBuilding(uint64 oid, int planet) {
	BuildingObject* buio = NULL;


	StringBuffer msg;
	msg << "Loading Static Building id: " << oid;
	info(msg.toString());

	StringBuffer query;
	query << "SELECT * FROM staticobjects WHERE zoneid = '" << planet << "' AND objectid = '" << oid << "';";

	try {
		ResultSet* result = ServerDatabase::instance()->executeQuery(query);

		if (result->next()) {
			uint64 oid = result->getUnsignedLong(1);
			uint64 parentId = result->getUnsignedLong(2);

			String file = result->getString(3);

			float oX = result->getFloat(4);
			float oY = result->getFloat(5);
			float oZ = result->getFloat(6);
			float oW = result->getFloat(7);

			float x = result->getFloat(8);
			float z = result->getFloat(9);
			float y = result->getFloat(10);

			float type = result->getFloat(11);
			bool client = result->getBoolean(12);

			/*if (file.indexOf("building") != -1 && file.indexOf("cloning") != -1) {
				CloningFacility* cloningFacility = new CloningFacility(oid, client);
				buio = (BuildingObject*) cloningFacility;

				if (cloningFacilityMap->put(oid, cloningFacility) != NULL) {
					error("Error CloningFacility already exists\n");
					raise(SIGSEGV);
				}
			} else {
				buio = new BuildingObject(oid, client);
			}

			buio->setZoneProcessServer(server);

			buio->setObjectCRC(file.hashCode());
			buio->setBuildingType(guessBuildingType(oid, file));*/
			//setObjectType
			//setObjectName

			info("trying to create " + file);

			buio = (BuildingObject*) objectManager->createObject(file.hashCode(), oid);

			buio->initializePosition(x, z, y);
			buio->setDirection(oX, oZ, oY, oW);
			buio->setStaticBuilding(true);
			//buio->setPersistent(true); // static = persistent - don't save
			//buio->setUpdated(false); // static = persistent

			buio->insertToZone(zone);

			/*if (buildingMap->put(oid, buio) != NULL) {
				error("Error CELL/BUILDING already exists\n");
				raise(SIGSEGV);
			}*/
		}

		delete result;
	} catch (DatabaseException& e) {
		error(e.getMessage());
	} catch (...) {
		error("unreported exception caught in PlanetManagerImplementation::loadStaticBuilding");
	}

	return buio;
}

