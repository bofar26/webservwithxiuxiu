/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RouterStatic.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xzhen <xzhen@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/15 12:27:51 by mipang            #+#    #+#             */
/*   Updated: 2026/08/13 21:57:05 by xzhen            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Router.hpp"

//handle the method GET
//new: when the index file is missing, a real directory with autoindex on is
//listed instead of answering 404
HttpResponse Router::handleGet(const HttpRequest& request) const
{
	std::string		filePath;
	//find the file with path
	filePath = buildFilePath(request.getPath());
	if (!fileExists(filePath))//if file don't exist
	{
		std::string				dirPath = buildDirPath(request.getPath());
		const LocationConfig	*location = findLocation(request.getPath());

		if (isDirectory(dirPath) && location != NULL && location->getAutoindex())
			return (buildAutoindexResponse(dirPath, request.getPath()));
		return (buildErrorResponse(404));
	}
	return (buildFileResponse(filePath));//if file exixts
}

//handle the method DELETE
HttpResponse Router::handleDelete(const HttpRequest& request) const
{
	HttpResponse	response;
	std::string		filePath;
	//find the file with path
	filePath = buildFilePath(request.getPath());
	if (isDirectory(filePath))//it's a directory
		return (buildErrorResponse(403));
	if (!fileExists(filePath))//if the file don't exist
		return (buildErrorResponse(404));
	if (unlink(filePath.c_str()) != 0)//delete it but failed
		return (buildErrorResponse(500));

	response.setStatus(204);
	response.setBody("");
	return (response);
}

//router received a post request, then call handlePost to handle it
HttpResponse Router::handlePost(const HttpRequest& request) const
{
	const LocationConfig	*location = findLocation(request.getPath());

	//no upload_store means this server does not accept uploads at all
	if (location == NULL || location->getUploadStore() == "")
		return (buildErrorResponse(403));

	std::string	fileName;
	std::string	fileData;

	//if fail to extract multipart to upload
	if (!extractMultipart(request.getBody(), request.getHeader("Content-Type"),
			fileName, fileData))
	{
		//then i assume the body of request is the contenu of the file to upload
		fileName = "upload";
		fileData = request.getBody();
	}
	//client uploads 0 bytes
	if (fileData.empty())
		return (buildErrorResponse(400));

	//shouldn't trust clients, in case if they want to write something in SECRET.txt
	if (fileName.find('/') != std::string::npos
		|| fileName.find("..") != std::string::npos || fileName.empty())
		fileName = "upload";
	//get the real path
	std::string	target = location->getUploadStore() + "/" + fileName;
	//if writing fileData to disk/upload_store fails
	if (!writeFile(target, fileData))
		return (buildErrorResponse(500));
	//handlePost should return a response to client to precise what kind of result
	HttpResponse	response;
	response.setStatus(201);
	response.setHeader("Content-Type", "text/plain");
	response.setBody("Created " + fileName + "\n");
	return (response);
}

//decide if this is a request which should be handled by CGI, true yes, false not
bool	Router::isCgiRequest(const HttpRequest& request, std::string& interpreter,
			std::string& scriptPath, std::string& queryString) const
{
	std::string	path = request.getPath();
	size_t		mark = path.find('?');

	queryString = "";
	if (mark != std::string::npos)
	{
		queryString = path.substr(mark + 1);
		path = path.substr(0, mark);
	}
	
	if (!isPathSafe(path))
		return (false);

	const LocationConfig	*location = findLocation(path);
	if (location == NULL)
		return (false);
	if (location->getRedirect() != "")//a redirected route never executes
		return (false);
	if (!location->isMethodAllowed(request.getMethod()))
		return (false);

	size_t	dot = path.find_last_of('.');
	if (dot == std::string::npos)
		return (false);
	//if .py isn't declared as a CGI
	interpreter = location->getCgiInterpreter(path.substr(dot));
	if (interpreter == "")//this extension is not declared as a CGI here
		return (false);

	scriptPath = buildFilePath(path);
	return (fileExists(scriptPath));
}

bool	Router::extractMultipart(const std::string& body, const std::string& contentType,
			std::string& fileName, std::string& fileData) const
{	//find "boundary=" in content of post request
	size_t	boundaryPos = contentType.find("boundary=");
	//find "multipart/form-data" in content of post request
	//content of request should have boundary+multipart/form-data at same time
	if (contentType.find("multipart/form-data") == std::string::npos
		|| boundaryPos == std::string::npos)
		return (false);

	//in a real HTTP multipart body, there's -- in boudary
	std::string	boundary = "--" + contentType.substr(boundaryPos + 9);
	size_t		partStart = body.find(boundary);//find the first boundary

	if (partStart == std::string::npos)
		return (false);
	partStart += boundary.length();//skip the boundary

	//find the end of header
	size_t	headerEnd = body.find("\r\n\r\n", partStart);
	if (headerEnd == std::string::npos)
		return (false);
	//extract the header
	std::string	partHeader = body.substr(partStart, headerEnd - partStart);
	size_t		namePos = partHeader.find("filename=\"");

	if (namePos == std::string::npos)
		return (false);
	namePos += 10;//skip filename="
	size_t	nameEnd = partHeader.find('"', namePos);//end of filename is "
	if (nameEnd == std::string::npos)
		return (false);
	fileName = partHeader.substr(namePos, nameEnd - namePos);//eg:filename=cat.png
	//find the real begin of filedata
	size_t	dataStart = headerEnd + 4;
	size_t	dataEnd = body.find(boundary, dataStart);
	
	if (dataEnd == std::string::npos)
		return (false);

	if (dataEnd >= 2 && body.compare(dataEnd - 2, 2, "\r\n") == 0)
		dataEnd -= 2;//delete the \r\n at the end of data
	fileData = body.substr(dataStart, dataEnd - dataStart);
	return (true);
}

HttpResponse	Router::buildFileResponse(const std::string& filePath) const
{
	HttpResponse response;

	response.setStatus(200);
	response.setHeader("Content-Type", getContentType(filePath));
	response.setBody(readFile(filePath));

	return (response);
}

const	LocationConfig* Router::findLocation(const std::string& path) const
{
	const std::vector<LocationConfig>& config_locations = _config.getLocation();
	for(size_t i = 0; i < config_locations.size(); i ++)
	{
		if (path.compare(0, config_locations[i].getPath().length(), config_locations[i].getPath()) == 0)
			return (&config_locations[i]);
	}
	return (NULL);
}
