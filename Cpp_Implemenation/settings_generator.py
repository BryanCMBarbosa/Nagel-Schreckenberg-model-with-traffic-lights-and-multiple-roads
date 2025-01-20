#!/usr/bin/env python3

import csv
import json
import math

def generate_json_from_csv_row(row):
    """
    Given a dict 'row' from the CSV (keys = column headers),
    return a Python dict representing the JSON 'simulation' object.

    If 'gridSize' == 0, we generate only one road (roadID=0) and no intersections.
    Otherwise, we generate N vertical + N horizontal roads with equally spaced intersections.
    """

    # 1) Parse main parameters from row
    # Adjust these column names to match your CSV exactly
    output_filename     = row["outputFilename"]
    episodes           = int(row["episodes"])
    queue_size         = int(row["queueSize"])
    controller_type    = row["controllerType"]
    cycle_time         = int(row["cycleTime"])
    v_max              = int(row["vMax"])
    brake_probability  = float(row["brakeProbability"])
    N                  = int(row["gridSize"])          # if N=0 => single road
    road_size          = int(row["roadSize"])
    # If 'isPeriodic' is in the CSV as "true"/"false"/"TRUE"/"FALSE"/"0"/"1",
    # parse accordingly:
    is_periodic_str    = row["isPeriodic"].strip().lower()
    is_periodic        = (is_periodic_str in ["true", "1", "yes"])
    alpha_weight       = float(row["alphaWeight"])
    beta               = float(row["beta"])
    density            = float(row["density"])
    prob_change        = float(row["probChange"])
    time_open          = float(row["timeOpen"])
    time_closed        = float(row["timeClosed"])

    # 2) Build the top-level "simulation" dict
    simulation_data = {
        "episodes": episodes,
        "queueSize": queue_size,
        "controllerType": controller_type,
        "cycleTime": cycle_time,
        "vMax": v_max,
        "brakeProbability": brake_probability,
        # "numberOfColumns" helps a controller interpret trafficLightGroups as a matrix
        # If N=0 (a single road), there's effectively no columns. You can choose 1 or 0:
        "numberOfColumns": (N if N > 0 else 1),

        "roads": [],
        "trafficLightGroups": [],
        "trafficLights": []
    }

    # We'll fill "roads", "trafficLightGroups", and "trafficLights" depending on N

    # --------------------------------------------------------------------------
    # Handle the special case: N == 0 => single road, no intersections.
    # --------------------------------------------------------------------------
    if N == 0:
        # Just make one road (ID=0)
        # If it's periodic, alpha=0.0, beta=0.0
        road_data = {
            "roadID": 0,
            "roadSize": road_size,
            "isPeriodic": is_periodic,
            "maxSpeed": v_max,
            "brakeProbability": brake_probability,
            "alphaWeight": (0.0 if is_periodic else alpha_weight),
            "beta": (0.0 if is_periodic else beta),
            "density": density,
            "sharedSections": []
        }
        simulation_data["roads"].append(road_data)

        # No traffic lights if there's only one road and no intersections
        simulation_data["trafficLightGroups"] = []
        simulation_data["trafficLights"] = []

    else:
        # ----------------------------------------------------------------------
        # Otherwise, build an N x N grid:
        #  - vertical roads (IDs 0..N-1)
        #  - horizontal roads (IDs N..2N-1)
        #  - N*N intersections
        # ----------------------------------------------------------------------

        roads = []
        # Create vertical roads (IDs 0..N-1)
        for i in range(N):
            rdata = {
                "roadID": i,
                "roadSize": road_size,
                "isPeriodic": is_periodic,
                "maxSpeed": v_max,
                "brakeProbability": brake_probability,
                "alphaWeight": (0.0 if is_periodic else alpha_weight),
                "beta": (0.0 if is_periodic else beta),
                "density": density,
                "sharedSections": []
            }
            roads.append(rdata)

        # Create horizontal roads (IDs N..2N-1)
        for i in range(N):
            rdata = {
                "roadID": N + i,
                "roadSize": road_size,
                "isPeriodic": is_periodic,
                "maxSpeed": v_max,
                "brakeProbability": brake_probability,
                "alphaWeight": (0.0 if is_periodic else alpha_weight),
                "beta": (0.0 if is_periodic else beta),
                "density": density,
                "sharedSections": []
            }
            roads.append(rdata)

        traffic_light_groups = []
        traffic_lights = []

        default_transition_time = 5  # or read from CSV if needed

        # Build intersections + traffic lights
        for r in range(N):
            horiz_road_id = N + r
            for c in range(N):
                vert_road_id = c
                group_id = r*N + c  # intersection group ID

                group_data = {
                    "groupID": group_id,
                    "transitionTime": default_transition_time
                }
                traffic_light_groups.append(group_data)

                # Intersection index in vertical road:
                vert_index = int(round((r + 0.5) / N * (road_size - 1)))
                # Intersection index in horizontal road:
                horiz_index = int(round((c + 0.5) / N * (road_size - 1)))

                # Add shared section to the vertical road c
                # roads array: vertical roads = roads[0..N-1], so index c is correct
                shared_entry = [
                    horiz_road_id,  # connected road ID
                    vert_index,     # current road index
                    horiz_index,    # connected road index
                    prob_change,    # prob vertical->horizontal
                    prob_change     # prob horizontal->vertical
                ]
                roads[vert_road_id]["sharedSections"].append(shared_entry)

                # If the controller is external, set timeOpen/timeClosed = -1
                external_control = (controller_type.lower() == "external")
                if external_control:
                    t_open = -1
                    t_closed = -1
                else:
                    t_open = time_open
                    t_closed = time_closed

                # Create 2 traffic lights, one for each road
                tl_vertical = {
                    "roadID": vert_road_id,
                    "position": vert_index,
                    "externalControl": external_control,
                    "timeOpen": t_open,
                    "timeClosed": t_closed,
                    "paired": True,
                    "groupID": group_id
                }
                traffic_lights.append(tl_vertical)

                tl_horizontal = {
                    "roadID": horiz_road_id,
                    "position": horiz_index,
                    "externalControl": external_control,
                    "timeOpen": t_open,
                    "timeClosed": t_closed,
                    "paired": True,
                    "groupID": group_id
                }
                traffic_lights.append(tl_horizontal)

        simulation_data["roads"] = roads
        simulation_data["trafficLightGroups"] = traffic_light_groups
        simulation_data["trafficLights"] = traffic_lights

    # Wrap in the final structure
    final_json = {"simulation": simulation_data}
    return final_json, output_filename


def main(csv_input_path):
    with open(csv_input_path, newline='') as csvfile:
        reader = csv.DictReader(csvfile)
        for row in reader:
            config_json, filename = generate_json_from_csv_row(row)
            # Write the JSON to the specified filename
            with open(filename, "w", encoding="utf-8") as f:
                json.dump(config_json, f, indent=2)
            print(f"Generated {filename}.")


if __name__ == "__main__":
    import sys
    if len(sys.argv) < 2:
        print("Usage: python generate_json.py <input.csv>")
        sys.exit(1)
    csv_input_path = sys.argv[1]
    main(csv_input_path)