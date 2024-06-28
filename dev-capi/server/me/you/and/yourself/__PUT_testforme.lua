local json = require("json")

-- Function to check if a string is JSON
local function isJSON(str)
    local success, result = pcall(json.decode, str)
    return success and type(result) == "table"
end

function test(...)
    local num_params = select("#", ...)
    
    if num_params ~= 3 then
        error("Function 'test' expects exactly 3 parameter")
    end

    -- Extract the first parameter
    local jsonBody = select(1, ...)

        -- Parse JSON to Lua table
    local json = require("json")
    local data = json.decode(jsonBody)
    
    -- Table to store key-value pairs for building the SQL query
    local keyValuePairs = {}

    -- Iterate through the table and build key-value pairs
    for key, value in pairs(data) do
        -- Assuming the SQL table has columns 'key' and 'value'
        -- Modify the column names as per your SQL table structure
        table.insert(keyValuePairs, {key, value})
    end

    -- Construct SET clause for SQL query
    local setClauses = {}
    for _, pair in ipairs(keyValuePairs) do
        table.insert(setClauses, string.format("%s = '%s'", pair[1], pair[2]))
    end

    -- Initialize table to hold query parts
    local query_parts = {}
    -- Print the parameters for debugging
    for i = 2, num_params do
        local param = select(i, ...)
        print(string.format("parameter %d = %s", i, param))
        -- Tokenize parameter using '=' as delimiter
        local delimiter = "="
        local tokens = tokenize(param, delimiter)

        -- Construct the WHERE clause of the SQL query using tokens
        if #tokens ~= 2 then
            error("Invalid parameter format")
        end
        -- Append column name and value to the query
        query_parts[i-1] = string.format("%s = '%s'", tokens[1], tokens[2])
    end

    -- Ensure all parts are valid
    if not query_parts[1] or not query_parts[2] then
        error("Invalid query parts")
    end
    
    -- Construct SQL query
    local query = string.format("UPDATE table_1 SET %s WHERE %s AND %s",
                                table.concat(setClauses, ", "), query_parts[1], query_parts[2])

    -- Call query_update_Lua function with the constructed query
    local result = query_update_Lua(query)
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

-- Function to tokenize a string
function tokenize(str, delimiter)
    local tokens = {}
    for token in string.gmatch(str, "([^" .. delimiter .. "]+)") do
        table.insert(tokens, token)
    end
    return tokens
end