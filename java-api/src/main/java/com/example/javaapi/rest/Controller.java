package com.example.javaapi.rest;

import com.example.javaapi.db.Table1;
import com.example.javaapi.services.Table1Service;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@RestController
@RequestMapping("/me/you/and/yourself")
public class Controller {

    @Autowired
    private Table1Service table1Service;

    @GetMapping("/all")
    public List<Table1> getAll() {
        return table1Service.findAll();
    }

    @GetMapping
    public List<Table1> getByColumns(
            @RequestParam(required = false) String column_a,
            @RequestParam(required = false) String column_b,
            @RequestParam(required = false) String column_c,
            @RequestParam(required = false) String column_d) {
        return table1Service.findByColumns(column_a, column_b, column_c, column_d);
    }

    @PostMapping(consumes = {MediaType.APPLICATION_JSON_VALUE, MediaType.APPLICATION_XML_VALUE})
    public ResponseEntity<Table1> create(@RequestBody Table1 table1) {
        Table1 savedTable1 = table1Service.save(table1);
        return new ResponseEntity<>(savedTable1, HttpStatus.CREATED);
    }

    @PutMapping
    public ResponseEntity<Void> updateByColumns(
            @RequestParam String columnA,
            @RequestParam String columnB,
            @RequestBody Table1 table1) {
        table1Service.updateByColumnAAndColumnB(columnA, columnB, table1.getColumnC(), table1.getColumnD());
        return ResponseEntity.noContent().build();
    }

    @DeleteMapping
    public ResponseEntity<Void> deleteByColumns(
            @RequestParam(required = false) String column_a,
            @RequestParam(required = false) String column_b,
            @RequestParam(required = false) String column_c,
            @RequestParam(required = false) String column_d) {
        table1Service.deleteByColumns(column_a, column_b, column_c, column_d);
        return ResponseEntity.noContent().build();
    }


}
