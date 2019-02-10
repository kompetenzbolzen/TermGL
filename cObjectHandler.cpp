#include "cObjectHandler.h"

cObjectHandler::cObjectHandler(cRender *_render)
{
	render = _render;

	cameraPosition = {0,0};

	iActiveObject = 0;

	objects.push_back(NULL); //Create first Object as Catcher for Events

	buildHitmap();
}

int cObjectHandler::createObject(cObject *_object)
{
	objects.push_back(_object);

	buildHitmap();
	return objects.size() - 1;
}

int cObjectHandler::moveObject(int _object, sPos _pos, int _mode)
{
	if (_object >= objects.size()) //prevent segmentation faults
		return 1;

	if (!objects[_object])
		return 1;

	sPos objPosition = objects[_object]->getPosition();

	if (_mode == _MOVE_RELATIVE)
		objects[_object]->setPosition(sPos{ objPosition.x + _pos.x, objPosition.y + _pos.y });
	else if (_mode == _MOVE_ABSOLUTE)
		objects[_object]->setPosition(_pos);

	buildHitmap();
	return 0;
}

int cObjectHandler::destroyObject(int _object)
{
	if(!objects[_object])
		return 1;

	delete objects[_object];
	objects[_object] = NULL;

	buildHitmap();
	return 0;
}

int cObjectHandler::write()
{
	render->clear();

	for (unsigned long int i = 0; i < meshes.size(); i++)
	{
		if(meshes[i])
		{
			moveWiremesh(i,{-cameraPosition.x, -cameraPosition.y, 0} ,_MOVE_RELATIVE);
			meshes[i]->write(render);
			moveWiremesh(i,{cameraPosition.x, cameraPosition.y, 0},_MOVE_RELATIVE);
		}
	}

	for (unsigned long int i = 0; i < objects.size(); i++)
	{
		if (objects[i]) // Check if objects[i] is existent
		{
			//Draw every Object
			sObject obj = objects[i]->getObject(); //get Object #i

			for (int o = 0; o < obj.sizeY; o++) { //y axis
				for (int p = 0; p < obj.sizeX; p++) { //x axis
					if (obj.cScreen[p][o]) { //Dont overwrite empty pixels
						sPos pos{ obj.pos.x + p - cameraPosition.x,
							 				obj.pos.y + o - cameraPosition.y };
						render->drawPoint(obj.cScreen[p][o], pos, true, obj.wColor[p][o]);
					}
				}
			}
		}
	}

	return 0;
}

int cObjectHandler::clickEvent(sPos _pos, unsigned int _button)
{
	if(_pos.x >= iHitMap.size())
		return 1;
	if(_pos.y >= iHitMap[_pos.x].size())
		return 1;


	if(objects[ iHitMap[_pos.x][_pos.y] ])
	{
		sPos rel_pos;
		sPos obj_pos = objects[ iHitMap[_pos.x][_pos.y] ]->getPosition();
		rel_pos.x = _pos.x - obj_pos.x + cameraPosition.x;
		rel_pos.y = _pos.y - obj_pos.y + cameraPosition.y;

		iActiveObject = iHitMap[_pos.x][_pos.y]; //Set active object
		objects[ iHitMap[_pos.x][_pos.y] ]->onClick(rel_pos, _button);
	}
	else
		return 1;

	return 0;
}

int cObjectHandler::charEvent(unsigned char _c)
{
	if(objects.size() > iActiveObject)
	{
		if(objects[iActiveObject])
		{
			objects[iActiveObject]->onChar(_c);
		}
		else
			return 1;
	}

	return 0;
}

void cObjectHandler::buildHitmap()
{
	//Rebuild 2D vector
	sPos size = render->getSize();

	vector<unsigned int> cp;

	while(size.y > cp.size())
	{
		cp.push_back(0);
	}

	while (size.x > iHitMap.size())
	{
		iHitMap.push_back(cp);
	}

	while (size.x <= iHitMap.size())
	{
		iHitMap.pop_back();
	}
	for(unsigned int x = 0; x < iHitMap.size(); x++)
	{
		for(unsigned int y = 0; y < iHitMap[x].size(); y++)
		{
			iHitMap[x][y] = 0;
		}
	}
	//Write object IDs to iHitMap
	for(unsigned int i = 0; i < objects.size(); i++)
	{
		if(objects[i])
		{
			sPos oPos = objects[i]->getPosition();
			sPos oSize = objects[i]->getSize();

			oPos.x -= cameraPosition.x;
			oPos.y -= cameraPosition.y;

			for(int x = oPos.x; x < oPos.x + oSize.x; x++)
			{
				for(int y = oPos.y; y < oPos.y + oSize.y; y++)
				{
					if((x < size.x && y < size.y) && (x >= 0 && y >= 0)) //Objects can be outside the screen.
						iHitMap[x][y] = i;
				}//for
			}//for
		}//if
	}//for
}//buildHitmap

void cObjectHandler::focusNext()
{
	iActiveObject++;

	if(iActiveObject >= objects.size())
		iActiveObject = 0;
}

void cObjectHandler::focus(unsigned int _id)
{
	if(_id >= objects.size())
		iActiveObject = objects.size();
	else
		iActiveObject = _id;
}

int cObjectHandler::createWiremesh(cWiremesh *_mesh)
{
	meshes.push_back(_mesh);

	return meshes.size() - 1;
}

int cObjectHandler::moveWiremesh(int _mesh, sCoord3d _pos, int _mode)
{
	if (_mesh >= meshes.size()) //prevent segmentation faults
		return 1;

	if (!meshes[_mesh])
		return 1;

	sCoord3d meshPosition = meshes[_mesh]->getPosition();

	if (_mode == _MOVE_RELATIVE)
		meshes[_mesh]->setPosition(meshPosition + _pos);
	else if (_mode == _MOVE_ABSOLUTE)
		meshes[_mesh]->setPosition(_pos);

	return 0;
}

int cObjectHandler::destroyWiremesh(int _mesh)
{
	if(!meshes[_mesh])
		return 1;

	delete meshes[_mesh];
	meshes[_mesh] = NULL;

	return 0;
}

int cObjectHandler::rotateWiremesh(int _mesh, sCoord3d _angle)
{
	if (_mesh >= meshes.size()) //prevent segmentation faults
		return 1;

	if (!meshes[_mesh])
		return 1;

	meshes[_mesh]->rotate(_angle);

	return 0;
}

void cObjectHandler::setCameraPosition(sPos _pos, int _mode)
{
	if(_mode == _MOVE_ABSOLUTE)
		cameraPosition = _pos;
	else if(_mode == _MOVE_RELATIVE)
	{
		cameraPosition.x += _pos.x;
		cameraPosition.y += _pos.y;
	}

	buildHitmap();
}

sPos cObjectHandler::getCameraPosition()
{
	return cameraPosition;
}
