//
//  ViewController.swift
//  SeatCanvasSample
//
//  Created by king on 2025/11/12.
//

import UIKit

class ViewController: UIViewController {
    var tableView: UITableView!

    var basemapNames: [String] = [
        "performbg",
        "performbg_2",
        "73807",
        "73808",
    ]

    override func viewDidLoad() {
        super.viewDidLoad()

        title = "底图"

        setupTableView()
    }
}

extension ViewController: UITableViewDataSource, UITableViewDelegate {
    func setupTableView() {
        tableView = UITableView(frame: .zero, style: .plain)
        tableView.backgroundColor = .white
        tableView.translatesAutoresizingMaskIntoConstraints = false
        tableView.register(UITableViewCell.classForCoder(), forCellReuseIdentifier: "cell")
        view.addSubview(tableView)

        NSLayoutConstraint.activate([
            tableView.leadingAnchor.constraint(equalTo: view.leadingAnchor),
            tableView.topAnchor.constraint(equalTo: view.safeAreaLayoutGuide.topAnchor),
            tableView.trailingAnchor.constraint(equalTo: view.trailingAnchor),
            tableView.bottomAnchor.constraint(equalTo: view.bottomAnchor),
        ])

        tableView.dataSource = self
        tableView.delegate = self
    }

    func tableView(_: UITableView, numberOfRowsInSection _: Int) -> Int {
        basemapNames.count
    }

    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        guard let cell = tableView.dequeueReusableCell(withIdentifier: "cell") else {
            fatalError("Unregistered cell")
        }

        cell.textLabel?.text = basemapNames[indexPath.row]
        return cell
    }

    func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        tableView.deselectRow(at: indexPath, animated: true)
        let name = basemapNames[indexPath.row]
        let vc = SeatCanvasViewController(basemapName: name)
        navigationController?.pushViewController(vc, animated: true)
    }
}
