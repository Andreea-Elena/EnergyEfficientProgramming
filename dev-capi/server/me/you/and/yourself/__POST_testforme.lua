-- Your existing code
local json = require("json")
local xml2lua = require("xml2lua")

-- Uses a handler that converts the XML to a Lua table
local handler = require("xmlhandler.tree")

-- Function to check if a string is JSON
local function isJSON(str)
    local success, result = pcall(json.decode, str)
    return success and type(result) == "table"
end

function test(...)
    local num_params = select("#", ...)
    
    if num_params ~= 1 then
        error("Function 'test' expects exactly 1 parameter")
    end

    -- Extract the first parameter
    local body = ...
    local data
    if isJSON(body) then
        -- Parse JSON to Lua table
        data = json.decode(body)
    else
        local parser = xml2lua.parser(handler)
        local success, errorMsg = pcall(parser.parse, parser, body)
        if not success then
            error("Failed to parse XML: " .. errorMsg)
        end
        data = handler.root
        
        -- Debug: print out the parsed XML
        print("Parsed XML:")
        print(json.encode(data))
    end
    
    -- Check if data is nil
    if data == nil then
        error("Failed to parse XML")
    end
    
    -- Table to store key-value pairs for building the SQL query
    local keyValuePairs = {}

    -- Iterate through the table and build key-value pairs
    for key, value in pairs(data) do
        -- Assuming the SQL table has columns 'key' and 'value'
        -- Modify the column names as per your SQL table structure
        table.insert(keyValuePairs, {key, value})
    end

    -- Construct SQL query
    local columns = {}
    local values = {}
    for _, pair in ipairs(keyValuePairs) do
        table.insert(columns, pair[1])
        table.insert(values, "'" .. pair[2] .. "'")
    end
    local query = string.format("INSERT INTO table_1 (%s) VALUES (%s)",
                                table.concat(columns, ", "),
                                table.concat(values, ", "))

    print(query)
    -- Call query_insert_Lua function with the constructed query
    local result = query_insert_Lua(query)
    local response
    if result ~= "FAILED" then 
        if isJSON(result) then
            response = format_response_Lua(200, "Content-Type: application/json", result)
        else
            response = format_response_Lua(200, "Content-Type: text/plain", result)
        end
    else 
        response = format_response_Lua(400, "Content-Type: text/html", "<html><body><h1>400 Bad Request</h1><p>Your request is invalid.</p></body></html>")
    end
    return response
end

