package de.marioehkart.marioehkarte;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.time.LocalDateTime;
import java.util.ArrayList; // import the ArrayList class

/// @author Mario Ehkart
/// @version 1.0
/// @since 2023-10-01
//! @brief This class represents a map of Carrera sections and their coordinates.
//! @details The Map class is used to create a map of the rail sections and their coordinates.
//! It includes methods for reading a CSV file containing section data,
//! getting coordinates for a specific section and distance, and handling errors.
//! @license GPL-3.0
//! @see Section
//! @see Rail
//! @see Coordinates

public class Map {
    /// @brief Constructor for the Map class.
    /// Initializes the map with a specified directory for error logging.
    /// @param directory The directory where the error log file will be created.
    /// @throws Exception If an error occurs while creating the error log file.
    /// @details The constructor initializes the sections and mappedSections arrays,
    /// and creates a FileWriter for logging errors.
    /// The error log file is created in the specified directory with the name
    /// "error.log".
    /// The sections array is used to store the sections of the map,
    /// and the mappedSections array is used to map section numbers to their
    /// corresponding indices in the sections array.
    /// The error log file is used to log any errors that occur during the execution
    /// of the program.
    public Map(String directory) throws Exception {
        sections = new ArrayList<Section>();
        sections.add(new Section(new Coordinates(), 0)); // Add a dummy section at index 0 for error handling
        mappedSections = new int[256];
        this.workDirectory = directory;
        errorWriter = new FileWriter(workDirectory + "/error.log", true);
        // Constructor
    }

    /// @brief Closes the error log file.
    /// @throws Exception If an error occurs while closing the error log file.
    /// @details This method is used to close the error log file after all
    /// operations are completed.
    /// It ensures that any remaining data in the buffer is written to the file
    /// before closing it.
    /// The error log file is used to log any errors that occur during the execution
    /// of the program.
    public void close() throws Exception {
        // Method to close the error log file
        errorWriter.close();
    }

    /// ! @brief Gets the coordinates for a specific section and distance.
    /// @param section The section number.
    /// @param distance The distance along the section.
    /// @return The coordinates for the specified section and distance.
    /// @throws Exception If an error occurs while getting the coordinates.
    /// @details This method retrieves the coordinates for a specific section and
    /// distance.
    /// It uses the mappedSections array to find the corresponding section index,
    /// and then calls the position method of the Section class to get the
    /// coordinates.
    public Coordinates getCoordinates(int section, double distance) throws Exception {
        Coordinates result = null;
        try {
            int sectionIndex = mappedSections[section];
            if (0 == sectionIndex) {
                throw new IllegalArgumentException("Tried to get Section " + section + ": not found.");
            }
            result = sections.get(section).position(distance);
        } catch (IllegalArgumentException e) {
            errorWriter.append(LocalDateTime.now() + ": Illegal Argument Exception: " + e.getMessage() + "\n");
            errorWriter.flush();
            System.out.println("IO Exception: " + e.getMessage());
        }

        // Method to get coordinates
        return result;
    }

    /// ! @brief Reads a CSV file and populates the map with sections and rails.
    /// @param csv The path to the CSV file.
    /// @param separator The separator used in the CSV file.
    /// @throws Exception If an error occurs while reading the CSV file.
    /// @details This method reads a CSV file containing section data and populates
    /// the map with sections and rails.
    public void feed(String csv, String separator) throws Exception {
        // Method to read CSV file
        BufferedReader csvReader = null;
        try {
            csvReader = new BufferedReader(new FileReader(csv));
            String line;
            line = csvReader.readLine(); // erste line überspringen
            int sectionCount = 0;
            while ((line = csvReader.readLine()) != null) {
                String[] values = line.split(separator);
                System.out.print(values[0]);
                if (values[0].equals("SECTION_START")) {
                    sectionCount++;
                    System.out.print(": " + values[1] + ", " + values[2] + " rails");
                    Coordinates sectionCoordinates = new Coordinates();
                    sectionCoordinates.x = Double.parseDouble(values[3]);
                    sectionCoordinates.y = Double.parseDouble(values[4]);
                    sectionCoordinates.angle_x = Math.toRadians(Double.parseDouble(values[5]));
                    sections.add(new Section(sectionCoordinates, Integer.parseInt(values[2])));
                    mappedSections[Integer.parseInt(values[1])] = sectionCount;
                } else if (values[0].equals("SECTION_END"))
                    ;
                else {
                    sections.get(sectionCount).addRail(Rail.fromString(values[0]));
                }
                System.out.println();
            }
            System.out.println(sections.size() + " sections imported from " + csv);
        } catch (FileNotFoundException e) {
            errorWriter.append(LocalDateTime.now() + ": File not found: " + csv + "\n");
            errorWriter.flush();
            System.out.println("File not found: " + csv);
        } catch (IOException e) {
            errorWriter.append(LocalDateTime.now() + ": IO Exception: " + e.getMessage() + "\n");
            errorWriter.flush();
            System.out.println("IO Exception: " + e.getMessage());
        } finally {
            if (csvReader != null) {
                try {
                    csvReader.close();
                } catch (IOException e) {
                    errorWriter.append(LocalDateTime.now() + ": IO Exception: " + e.getMessage() + "\n");
                    errorWriter.flush();
                    System.out.println("IO Exception: " + e.getMessage());
                }
            }
        }

    }

    private FileWriter errorWriter;
    private String workDirectory;
    private ArrayList<Section> sections; // Create an ArrayList object
    private int[] mappedSections; // Create an array of doubles

}
