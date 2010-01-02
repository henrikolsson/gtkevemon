<?php
/*
 * This file is part of GtkEveMon Fitter.
 *
 * GtkEveMon is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.
 */

// Config
$formatVersion = 1;
$dataVersion = 1;

// Database connection
mysql_connect("localhost", "user", "pass");
mysql_select_db("database");

/**
 *Recursive function to generate all the tags below the first level market group
 *
 * @global <DOMDocument> $itemTreeXML need this object to create DOMElements and DOMAttributes
 * @param <int> $marketGroupID ID of the group we are going through now
 * @param <DOMElement> $parentElement DOMElement which is the upper level ItemCategory
 */
function printCategoryXML($marketGroupID, $parentElement)
{
    global $itemTreeXML;

    // Query to get the current category (market group) and its icon
    $categoryQuery = "SELECT img.marketGroupID, img.marketGroupName, img.description, img.graphicID, eg.icon
                   FROM invMarketGroups img
                   LEFT JOIN eveGraphics eg ON img.graphicID = eg.graphicID
                   WHERE img.marketGroupID = %d";

    /*
     * Here we get the information about the current category and create
     * the ItemCategory tag with its attributes.
     * The parent is either a first level market group or one of its subcategories.
     */
    $categoryResult = mysql_query(sprintf($categoryQuery, $marketGroupID));

    $categoryRow = mysql_fetch_assoc($categoryResult);

    $itemCategory = $parentElement->appendChild($itemTreeXML->createElement("ItemCategory"));

    $id = $itemCategory->appendChild($itemTreeXML->createAttribute("id"));
    $id->appendChild($itemTreeXML->createTextNode($marketGroupID));

    $name = $itemCategory->appendChild($itemTreeXML->createAttribute("name"));
    $name->appendChild($itemTreeXML->createTextNode($categoryRow["marketGroupName"]));

    $graphicsIcon = $itemCategory->appendChild($itemTreeXML->createAttribute("icon"));
    $graphicsIcon->appendChild($itemTreeXML->createTextNode($categoryRow["icon"]));

    $description = $itemCategory->appendChild($itemTreeXML->createElement("Description"));
    $description->appendChild($itemTreeXML->createTextNode($categoryRow["description"]));

    // Query to get the subcategories of the current category
    $subCategoriesQuery = "SELECT marketGroupID, parentGroupID FROM invMarketGroups WHERE parentGroupID = %d";

    // Get the subcategories and recursivly call this function
    $subCategoriesResult = mysql_query(sprintf($subCategoriesQuery, $marketGroupID));
    if(mysql_num_rows($subCategoriesResult) > 0)
    {
        $subCategories = $itemCategory->appendChild($itemTreeXML->createElement("SubCategories"));

        while($subCategoriesRow = mysql_fetch_assoc($subCategoriesResult))
        {
            printCategoryXML($subCategoriesRow["marketGroupID"], $subCategories);
        }
    }

    // Query for the items of this category
    $itemQuery = "SELECT it.typeID, it.groupID, it.typeName, it.description, it.graphicID, it.radius, it.mass, it.volume,
                   it.capacity, it.portionSize, it.raceID, it.basePrice, eg.icon
               FROM invTypes it
               LEFT JOIN eveGraphics eg ON it.graphicID = eg.graphicID
               WHERE it.published = 1 AND it.marketGroupID = %d";

    // Get the items in the current category
    $itemResult = mysql_query(sprintf($itemQuery, $marketGroupID));

    if(mysql_num_rows($itemResult) > 0)
    {
        /*
         * Here we add the ItemList and the while loop adds the items.
         * The inner while loop adds additional attributes, which are not
         * the same for all items (e.g. explosiveDamage, etc.)
         */

        $itemList  = $itemCategory->appendChild($itemTreeXML->createElement("ItemList"));

        while($itemRow = mysql_fetch_assoc($itemResult))
        {
            $item = $itemList->appendChild($itemTreeXML->createElement("Item"));
 
            $properties = $item->appendChild($itemTreeXML->createElement("Properties"));

            $id = $item->appendChild($itemTreeXML->createAttribute("id"));
            $id->appendChild($itemTreeXML->createTextNode($itemRow["typeID"]));

            $name = $item->appendChild($itemTreeXML->createAttribute("name"));
            $name->appendChild($itemTreeXML->createTextNode($itemRow["typeName"])); 

            $icon = $item->appendChild($itemTreeXML->createAttribute("icon"));
            $icon->appendChild($itemTreeXML->createTextNode($itemRow["icon"]));
			
			if($itemRow["description"] != null && $itemRow["description"] != "")
			{
				$description = $item->appendChild($itemTreeXML->createElement("Description"));
				$description->appendChild($itemTreeXML->createTextNode(utf8_encode($itemRow["description"])));
            }

            if(is_numeric($itemRow["radius"]) && $itemRow["radius"] != 0)
			{
				$property = $properties->appendChild($itemTreeXML->createElement("Property", $itemRow["radius"])); 
				
				$name = $property->appendChild($itemTreeXML->createAttribute("name"));
                $name->appendChild($itemTreeXML->createTextNode("radius"));
			}
			
			if(is_numeric($itemRow["mass"]) && $itemRow["mass"] != 0)
			{
				$property = $properties->appendChild($itemTreeXML->createElement("Property", $itemRow["mass"])); 
				
				$name = $property->appendChild($itemTreeXML->createAttribute("name"));
                $name->appendChild($itemTreeXML->createTextNode("mass"));
			}

			if(is_numeric($itemRow["volume"]) && $itemRow["volume"] != 0)
			{
				$property = $properties->appendChild($itemTreeXML->createElement("Property", $itemRow["volume"])); 
				
				$name = $property->appendChild($itemTreeXML->createAttribute("name"));
                $name->appendChild($itemTreeXML->createTextNode("volume"));
			}

			if(is_numeric($itemRow["capacity"]) && $itemRow["capacity"] != 0)
			{
				$property = $properties->appendChild($itemTreeXML->createElement("Property", $itemRow["capacity"])); 
				
				$name = $property->appendChild($itemTreeXML->createAttribute("name"));
                $name->appendChild($itemTreeXML->createTextNode("capacity"));
			}

			if(is_numeric($itemRow["portionSize"]) && $itemRow["portionSize"] != 0)
			{
				$property = $properties->appendChild($itemTreeXML->createElement("Property", $itemRow["portionSize"])); 
				
				$name = $property->appendChild($itemTreeXML->createAttribute("name"));
                $name->appendChild($itemTreeXML->createTextNode("portionSize"));
			}

			if(is_numeric($itemRow["raceID"]) && $itemRow["raceID"] != 0)
			{
				$property = $properties->appendChild($itemTreeXML->createElement("Property", $itemRow["raceID"])); 
				
				$name = $property->appendChild($itemTreeXML->createAttribute("name"));
                $name->appendChild($itemTreeXML->createTextNode("raceID"));
			}

			if(is_numeric($itemRow["basePrice"]) && $itemRow["basePrice"] != 0)
			{
				$property = $properties->appendChild($itemTreeXML->createElement("Property", $itemRow["basePrice"])); 
				
				$name = $property->appendChild($itemTreeXML->createAttribute("name"));
                $name->appendChild($itemTreeXML->createTextNode("basePrice"));
			}

            // Query to get the item attributes
            $propertiesQuery = "SELECT dta.attributeID, dat.attributeName, COALESCE(dta.valueInt, dta.valueFloat) AS value
                       FROM dgmTypeAttributes dta
                       JOIN dgmAttributeTypes dat ON dta.attributeID = dat.attributeID
                       WHERE dta.typeID = %d";

            $propertiesResult = mysql_query(sprintf($propertiesQuery, $itemRow["typeID"]));

			//associative array to hold skills
			$skills = array();

            if(mysql_num_rows($propertiesResult) > 0)
            {

                while($propertiesRow = mysql_fetch_assoc($propertiesResult))
                {
					$matches = array();
					if(preg_match("/^requiredSkill([0-9]*)/", $propertiesRow["attributeName"], $matches))
					{
						$skillNum = $matches[1];
						if(!is_array($skills[$skillNum]))
						{
							$skills[$skillNum] = array();
						}
						//wether we're dealing with skill id or level
						if(substr($propertiesRow["attributeName"], -5) == "Level")
						{
							$skills[$skillNum]["level"] = $propertiesRow["value"];
						}
						else
						{
							$skills[$skillNum]["id"] = $propertiesRow["value"];
						}
					}
					//We're dealing with a normal attribute
					else
					{
						$property = $properties->appendChild($itemTreeXML->createElement("Property", $propertiesRow["value"])); 

						$name = $property->appendChild($itemTreeXML->createAttribute("name"));
						$name->appendChild($itemTreeXML->createTextNode($propertiesRow["attributeName"]));
                    } 
                }
			
				//Handle skills
				if(count($skills) > 0)
				{
					$requiredSkills = $item->appendChild($itemTreeXML->createElement("RequiredSkills"));

					foreach ($skills as $currentSkill)
					{
						$skill = $requiredSkills->appendChild($itemTreeXML->createElement("Skill")); 
						
						$id = $skill->appendChild($itemTreeXML->createAttribute("id"));
						$id->appendChild($itemTreeXML->createTextNode($currentSkill["id"]));
						
						$level = $skill->appendChild($itemTreeXML->createAttribute("level"));
						$level->appendChild($itemTreeXML->createTextNode($currentSkill["level"]));
					}
				}
            }
        }
    }
}

// Create the DOM document which will be our XML data and create the itemDB tag
$itemTreeXML = new DOMDocument("1.0");

$parentElement = $itemTreeXML->appendChild($itemTreeXML->createElement("ItemDB"));
$itemTreeXML->formatOutput = true;

$version = $parentElement->appendChild($itemTreeXML->createAttribute("version"));
$version->appendChild($itemTreeXML->createTextNode($formatVersion));

$dVersion = $parentElement->appendChild($itemTreeXML->createAttribute("dataVersion"));
$dVersion->appendChild($itemTreeXML->createTextNode($dataVersion));

// Query for the root market groups
$invMarketGroupsQuery = "SELECT marketGroupID FROM invMarketGroups WHERE parentGroupID IS NULL";

/*
 * We execute the query above and jump into the first recursion level with the
 * root market groups.
 */
$invMarketGroupsResult = mysql_query($invMarketGroupsQuery);
while($invMarketGroupsRow = mysql_fetch_assoc($invMarketGroupsResult))
{
    printCategoryXML($invMarketGroupsRow["marketGroupID"], $parentElement);
}

// Open the file we want to save the XML data to and write the data
$file = fopen("ItemTree.xml", "w");
fwrite($file, $itemTreeXML->saveXML());
fclose($file);
?>
