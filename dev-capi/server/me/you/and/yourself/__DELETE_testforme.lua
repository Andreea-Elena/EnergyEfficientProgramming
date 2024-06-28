local json = require("json")

-- Function to check if a string is JSON
local function isJSON(str)
    local success, result = pcall(json.decode, str)
    return success and type(result) == "table"
end

function test(...)
    local num_params = select("#", ...)
    if num_params ~= 4 then
        error("Function 'test' expects exactly 4 parameters")
    end

    -- Initialize table to hold query parts
    local query_parts = {}
    -- Print the parameters for debugging
    for i = 1, num_params do
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
        query_parts[i] = string.format("%s = '%s'", tokens[1], tokens[2])
    end

    -- Ensure all parts are valid
    if not query_parts[1] or not query_parts[2] or not query_parts[3] or not query_parts[4] then
        error("Invalid query parts")
    end

    -- Construct the full SQL query
    local query = string.format("DELETE FROM table_1 WHERE %s AND %s AND %s AND %s;",
                                query_parts[1], query_parts[2], query_parts[3], query_parts[4])

    -- Call query_delete_Lua function with the constructed query
    local result = query_delete_Lua(query)
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