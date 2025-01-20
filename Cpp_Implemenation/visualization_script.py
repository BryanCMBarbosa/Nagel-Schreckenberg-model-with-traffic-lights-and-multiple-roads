import json
import matplotlib.pyplot as plt
import argparse
import os

def load_simulation_results(filename):
    with open(filename, 'r') as f:
        data = json.load(f)
    return data

def plot_space_time_diagram(results, road_id=0, max_timesteps=100):
    episodes = results["episodes"]
    space_time = []
    
    for ep_index, ep_data in enumerate(episodes[:max_timesteps]):
        roads = ep_data.get("roads", [])
        road_dict = None
        for rd in roads:
            if rd.get("roadID") == road_id:
                road_dict = rd
                break
        if not road_dict:
            continue
        
        representation = road_dict.get("roadRepresentation", [])
        row = []
        for val in representation:
            if val == -1:
                row.append('.')  # empty
            else:
                row.append(str(val))  # show the speed
        space_time.append(row)
    
    fig, ax = plt.subplots(figsize=(10, 6))
    ax.set_title(f"Space-Time Diagram (road {road_id})")
    ax.set_xlabel("Space (cell index)")
    ax.set_ylabel("Time (step)")
    
    for t, row_data in enumerate(space_time):
        ax.text(0, t, ''.join(row_data))
    
    ax.set_ylim(len(space_time), 0)  # invert y-axis so time goes downward
    ax.set_xlim(0, max(len(r) for r in space_time) if space_time else 0)
    plt.show()

def plot_flow_vs_density(results, road_id=None):
    episodes = results["episodes"]
    densities = []
    flows = []
    
    for ep_data in episodes:
        roads = ep_data.get("roads", [])
        
        total_flow = 0.0
        total_density = 0.0
        count = 0
        
        for rd in roads:
            if road_id is not None and rd.get("roadID") != road_id:
                continue
            
            rho = rd.get("generalDensity", 0.0)
            q = rd.get("cumulativeTimeSpaceAveragedFlow", 0.0)
            
            total_flow += q
            total_density += rho
            count += 1
        
        if count > 0:
            mean_flow = total_flow / count
            mean_density = total_density / count
            densities.append(mean_density)
            flows.append(mean_flow)
    
    fig, ax = plt.subplots()
    ax.scatter(densities, flows, marker='o', alpha=0.5, label='Data')
    
    ax.set_xlabel("Density [cars per site]")
    ax.set_ylabel("Flow [cars per time step]")
    title = "Fundamental Diagram (Flow vs Density)"
    if road_id is not None:
        title += f" - Road {road_id}"
    else:
        title += " - (All Roads Combined)"
    ax.set_title(title)
    ax.legend()
    
    plt.show()

def plot_flow_vs_density_multiple_files(json_filepaths, road_id=None):
    densities = []
    flows = []
    labels = []

    for filepath in json_filepaths:
        with open(filepath, 'r') as f:
            results = json.load(f)

        episodes = results.get("episodes", [])
        if not episodes:
            print(f"Warning: No episodes found in {filepath}, skipping.")
            continue

        total_flow_over_time = 0.0
        total_density_over_time = 0.0
        episode_count = 0

        for ep_data in episodes:
            roads = ep_data.get("roads", [])
            
            sum_flow_this_step = 0.0
            sum_density_this_step = 0.0
            road_count_this_step = 0

            for rd in roads:
                if (road_id is not None) and (rd.get("roadID") != road_id):
                    continue
                
                rho = rd.get("generalDensity", 0.0)
                q   = rd.get("cumulativeTimeSpaceAveragedFlow", 0.0)
                
                sum_flow_this_step += q
                sum_density_this_step += rho
                road_count_this_step += 1
            
            if road_count_this_step > 0:
                mean_flow_step = sum_flow_this_step / road_count_this_step
                mean_density_step = sum_density_this_step / road_count_this_step

                total_flow_over_time += mean_flow_step
                total_density_over_time += mean_density_step
                episode_count += 1

        if episode_count > 0:
            avg_flow_sim = total_flow_over_time / episode_count
            avg_density_sim = total_density_over_time / episode_count

            flows.append(avg_flow_sim)
            densities.append(avg_density_sim)
            labels.append(filepath)

    fig, ax = plt.subplots(figsize=(7, 5))
    ax.scatter(densities, flows, marker='o', alpha=0.7)

    for (x, y, label) in zip(densities, flows, labels):
        ax.text(x, y, label, fontsize=8, ha='left', va='bottom')

    ax.set_xlabel("Density [cars per site]")
    ax.set_ylabel("Flow [cars per time step]")
    title = "Fundamental Diagram (Flow vs Density) - Multiple Simulations"
    if road_id is not None:
        title += f" (Road {road_id})"
    else:
        title += " (All Roads)"
    ax.set_title(title)
    plt.grid(True)
    plt.show()

def main():
    parser = argparse.ArgumentParser(
        description="Plot selected graphs for simulation JSON results."
    )
    parser.add_argument(
        '--json_dir',
        required=True,
        help='Directory containing JSON result files.'
    )
    parser.add_argument(
        '--plots',
        nargs='+',
        required=True,
        choices=['space_time_diagram', 'flow_vs_density', 'flow_vs_density_multiple'],
        help='List of plots to generate.'
    )
    parser.add_argument(
        '--road_id',
        type=int,
        default=0,
        help='Road ID to filter for certain plots (default: 0).'
    )
    parser.add_argument(
        '--max_timesteps',
        type=int,
        default=100,
        help='Maximum timesteps for space-time diagram (default: 100).'
    )
    args = parser.parse_args()

    # Collect all JSON file paths from the specified directory
    json_filepaths = [
        os.path.join(args.json_dir, f)
        for f in os.listdir(args.json_dir)
        if f.endswith('.json')
    ]

    if not json_filepaths:
        print(f"No JSON files found in directory: {args.json_dir}")
        return

    for filepath in json_filepaths:
        results = load_simulation_results(filepath)

        if 'space_time_diagram' in args.plots:
            plot_space_time_diagram(results, road_id=args.road_id, max_timesteps=args.max_timesteps)
        if 'flow_vs_density' in args.plots:
            plot_flow_vs_density(results, road_id=args.road_id)
        if 'flow_vs_density_multiple' in args.plots:
            plot_flow_vs_density_multiple_files(json_filepaths, road_id=args.road_id)

if __name__ == "__main__":
    main()
#python plot_simulation.py --json_dir ./results --plots flow_vs_density space_time_diagram --road_id 0 --max_timesteps 200
